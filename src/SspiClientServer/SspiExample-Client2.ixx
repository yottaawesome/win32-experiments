module;

// C++26
#define WIN32_LEAN_AND_MEAN
#define SECURITY_WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <schannel.h>
#include <security.h>
#include <secext.h>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

export module SspiExample:Client2;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "Crypt32.lib")

export {
	// A small helper to print and exit on failures
	void Fail(const char* where, SECURITY_STATUS ss) {
		std::cerr << where << " failed: 0x" << std::hex << ss << std::dec << "\n";
		std::exit(1);
	}
	void FailWSA(const char* where) {
		std::cerr << where << " failed: WSA " << WSAGetLastError() << "\n";
		std::exit(1);
	}

	SOCKET ConnectTcp(const char* host, const char* port) {
		addrinfo hints{};
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		addrinfo* res = nullptr;
		if (getaddrinfo(host, port, &hints, &res) != 0) FailWSA("getaddrinfo");
		SOCKET s = INVALID_SOCKET;
		for (auto p = res; p; p = p->ai_next) {
			s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
			if (s == INVALID_SOCKET) continue;
			if (connect(s, p->ai_addr, (int)p->ai_addrlen) == 0) break;
			closesocket(s);
			s = INVALID_SOCKET;
		}
		freeaddrinfo(res);
		if (s == INVALID_SOCKET) FailWSA("connect");
		return s;
	}

	int SendAll(SOCKET s, const void* buf, int len) {
		const char* p = (const char*)buf;
		int total = 0;
		while (total < len) {
			int n = send(s, p + total, len - total, 0);
			if (n <= 0) return n;
			total += n;
		}
		return total;
	}

	// Receive from socket into a dynamic buffer (non-blocking semantics simplified)
	int RecvSome(SOCKET s, std::vector<char>& buf) {
		char tmp[16 * 1024];
		int n = recv(s, tmp, sizeof(tmp), 0);
		if (n > 0) buf.insert(buf.end(), tmp, tmp + n);
		return n;
	}

	// Find a complete TLS record from accumulated data. SChannel tolerates partial buffers,
	// but providing whole records is efficient. We’ll just pass whatever we have.
	void BufferFromVec(std::vector<char>& src, SecBuffer& out, size_t maxBytes = SIZE_MAX) {
		size_t n = (std::min)(maxBytes, src.size());
		out.pvBuffer = n ? src.data() : nullptr;
		out.cbBuffer = (unsigned long)n;
		out.BufferType = SECBUFFER_TOKEN;
	}

	// Remove consumed bytes from the front of a vector
	void ConsumeFront(std::vector<char>& v, size_t n) {
		if (n == 0) return;
		if (n >= v.size()) { v.clear(); return; }
		v.erase(v.begin(), v.begin() + n);
	}

	// Encrypt application data using EncryptMessage
	std::vector<char> SslEncrypt(CtxtHandle& hCtx, const SecPkgContext_StreamSizes& sizes, const void* data, size_t len) {
		std::vector<char> out;
		size_t maxChunk = sizes.cbMaximumMessage;
		const BYTE* p = (const BYTE*)data;
		while (len) {
			size_t chunk = (std::min)(len, maxChunk);
			out.resize(out.size() + sizes.cbHeader + chunk + sizes.cbTrailer);
			BYTE* base = (BYTE*)out.data() + (out.size() - (sizes.cbHeader + chunk + sizes.cbTrailer));
			SecBuffer bufs[3];
			bufs[0].BufferType = SECBUFFER_STREAM_HEADER; bufs[0].pvBuffer = base;                         bufs[0].cbBuffer = sizes.cbHeader;
			bufs[1].BufferType = SECBUFFER_DATA;          bufs[1].pvBuffer = base + sizes.cbHeader;         bufs[1].cbBuffer = (unsigned long)chunk;
			bufs[2].BufferType = SECBUFFER_STREAM_TRAILER; bufs[2].pvBuffer = base + sizes.cbHeader + chunk; bufs[2].cbBuffer = sizes.cbTrailer;
			memcpy(bufs[1].pvBuffer, p, chunk);
			SecBufferDesc desc{ SECBUFFER_VERSION, 3, bufs };
			SECURITY_STATUS ss = EncryptMessage(&hCtx, 0, &desc, 0);
			if (ss != SEC_E_OK) Fail("EncryptMessage", ss);
			// bufs[1].cbBuffer may be shortened (rare). Respect actual sizes when sending:
			size_t produced = bufs[0].cbBuffer + bufs[1].cbBuffer + bufs[2].cbBuffer;
			// Trim if SChannel adjusted sizes
			out.resize(out.size() - (sizes.cbHeader + chunk + sizes.cbTrailer) + produced);
			p += chunk; len -= chunk;
		}
		return out;
	}

	// Decrypt using DecryptMessage and return plaintext appended to dst.
	// Handles extra data by keeping it in inBuf.
	SECURITY_STATUS SslDecrypt(CtxtHandle& hCtx, std::vector<char>& inBuf, std::vector<char>& dst) {
		SecBuffer bufs[4]{};
		BufferFromVec(inBuf, bufs[0]);
		bufs[1].BufferType = SECBUFFER_EMPTY;
		bufs[2].BufferType = SECBUFFER_EMPTY;
		bufs[3].BufferType = SECBUFFER_EMPTY;
		SecBufferDesc desc{ SECBUFFER_VERSION, 4, bufs };
		SECURITY_STATUS ss = DecryptMessage(&hCtx, &desc, 0, nullptr);
		if (ss == SEC_E_INCOMPLETE_MESSAGE) return ss; // need more data
		if (ss == SEC_I_CONTEXT_EXPIRED) return ss;    // close_notify
		if (ss != SEC_E_OK && ss != SEC_I_RENEGOTIATE) Fail("DecryptMessage", ss);

		// Find DATA and EXTRA
		BYTE* dataPtr = nullptr; DWORD dataLen = 0; DWORD extraLen = 0; BYTE* extraPtr = nullptr;
		for (int i = 0; i < 4; ++i) {
			if (bufs[i].BufferType == SECBUFFER_DATA) {
				dataPtr = (BYTE*)bufs[i].pvBuffer; dataLen = bufs[i].cbBuffer;
			}
			else if (bufs[i].BufferType == SECBUFFER_EXTRA) {
				extraPtr = (BYTE*)bufs[i].pvBuffer; extraLen = bufs[i].cbBuffer;
			}
		}
		if (dataPtr && dataLen) dst.insert(dst.end(), dataPtr, dataPtr + dataLen);

		// Preserve extra at front of inBuf
		if (extraLen) {
			size_t consumed = inBuf.size() - extraLen;
			ConsumeFront(inBuf, consumed);
		}
		else {
			inBuf.clear();
		}
		return ss;
	}

	// Validate server certificate using SChannel policy
	void VerifyServerCert(CtxtHandle& hCtx, const wchar_t* serverName) {
		PCCERT_CONTEXT pCert = nullptr;
		SECURITY_STATUS ss = QueryContextAttributesW(&hCtx, SECPKG_ATTR_REMOTE_CERT_CONTEXT, &pCert);
		if (ss != SEC_E_OK || !pCert) Fail("QueryContextAttributesW(REMOTE_CERT_CONTEXT)", ss);

		CERT_CHAIN_PARA chainPara{};
		chainPara.cbSize = sizeof(chainPara);
		PCCERT_CHAIN_CONTEXT pChain = nullptr;
		if (!CertGetCertificateChain(nullptr, pCert, nullptr, pCert->hCertStore, &chainPara, 0, nullptr, &pChain)) {
			CertFreeCertificateContext(pCert);
			Fail("CertGetCertificateChain", GetLastError());
		}

		SSL_EXTRA_CERT_CHAIN_POLICY_PARA sslExtra{};
		sslExtra.cbSize = sizeof(sslExtra);
		sslExtra.dwAuthType = AUTHTYPE_SERVER;
		sslExtra.fdwChecks = 0;
		sslExtra.pwszServerName = (LPWSTR)serverName;

		CERT_CHAIN_POLICY_PARA policyPara{};
		policyPara.cbSize = sizeof(policyPara);
		policyPara.pvExtraPolicyPara = &sslExtra;

		CERT_CHAIN_POLICY_STATUS policyStatus{};
		policyStatus.cbSize = sizeof(policyStatus);

		if (!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, pChain, &policyPara, &policyStatus)) {
			CertFreeCertificateChain(pChain);
			CertFreeCertificateContext(pCert);
			Fail("CertVerifyCertificateChainPolicy", GetLastError());
		}
		if (policyStatus.dwError) {
			CertFreeCertificateChain(pChain);
			CertFreeCertificateContext(pCert);
			Fail("Server certificate validation", policyStatus.dwError);
		}

		CertFreeCertificateChain(pChain);
		CertFreeCertificateContext(pCert);
	}

	// Perform a client TLS 1.3 handshake over an already-connected TCP socket
	void TlsHandshake(SOCKET s, const wchar_t* serverName,
		CredHandle& hCred, CtxtHandle& hCtx, bool& negotiatedTls13,
		SecPkgContext_StreamSizes& streamSizes) {
		SCHANNEL_CRED sc{};
		sc.dwVersion = SCHANNEL_CRED_VERSION;
		sc.grbitEnabledProtocols = SP_PROT_TLS1_3_CLIENT; // request TLS 1.3
		// Optionally restrict ciphersuites with SCH_CREDENTIALS if needed.
		sc.dwFlags =
			SCH_CRED_MANUAL_CRED_VALIDATION |   // we’ll validate after handshake
			SCH_USE_STRONG_CRYPTO |
			SCH_CRED_NO_DEFAULT_CREDS;

		TimeStamp ts{};
		SECURITY_STATUS ss = AcquireCredentialsHandleW(
			nullptr, const_cast<wchar_t*>(UNISP_NAME_W), SECPKG_CRED_OUTBOUND,
			nullptr, &sc, nullptr, nullptr, &hCred, &ts);
		if (ss != SEC_E_OK) Fail("AcquireCredentialsHandle", ss);

		SecBuffer outBuf{ 0, SECBUFFER_TOKEN, nullptr };
		SecBufferDesc outDesc{ SECBUFFER_VERSION, 1, &outBuf };

		DWORD ctxReq =
			ISC_REQ_ALLOCATE_MEMORY |
			ISC_REQ_CONFIDENTIALITY |
			ISC_REQ_STREAM;
		DWORD ctxAttr = 0;

		PCtxtHandle phCtx = nullptr;
		std::vector<char> inBuffer;     // raw TLS records from socket
		bool handshakeDone = false;

		while (!handshakeDone) {
			// Prepare input buffers (if we have data)
			SecBuffer inSecBufs[2]{};
			inSecBufs[0].BufferType = SECBUFFER_TOKEN;
			inSecBufs[0].pvBuffer = inBuffer.empty() ? nullptr : inBuffer.data();
			inSecBufs[0].cbBuffer = (unsigned long)inBuffer.size();
			inSecBufs[1].BufferType = SECBUFFER_EMPTY;
			SecBufferDesc inDesc{ SECBUFFER_VERSION, 2, inSecBufs };

			outBuf.pvBuffer = nullptr;
			outBuf.cbBuffer = 0;
			outBuf.BufferType = SECBUFFER_TOKEN;

			ss = InitializeSecurityContextW(
				&hCred, phCtx, const_cast<wchar_t*>(serverName),
				ctxReq, 0, SECURITY_NATIVE_DREP,
				phCtx ? &inDesc : nullptr, 0, &hCtx,
				&outDesc, &ctxAttr, &ts);

			if (ss == SEC_E_INCOMPLETE_MESSAGE) {
				// Need more data from server
				std::vector<char> rx;
				int n = RecvSome(s, inBuffer);
				if (n == 0) Fail("Handshake recv (connection closed)", SEC_E_INTERNAL_ERROR);
				if (n < 0) FailWSA("recv");
				continue;
			}

			if (outBuf.cbBuffer != 0 && outBuf.pvBuffer != nullptr) {
				// Send handshake data to server
				if (SendAll(s, outBuf.pvBuffer, (int)outBuf.cbBuffer) <= 0) FailWSA("send(handshake)");
				FreeContextBuffer(outBuf.pvBuffer);
				outBuf.pvBuffer = nullptr;
				outBuf.cbBuffer = 0;
			}

			if (ss == SEC_E_OK) {
				handshakeDone = true;
				break;
			}
			else if (ss == SEC_I_CONTINUE_NEEDED || ss == SEC_I_INCOMPLETE_CREDENTIALS) {
				// Might also need to consume any extra input the SSPI used
				// Find SECBUFFER_EXTRA to retain
				if (inSecBufs[1].BufferType == SECBUFFER_EXTRA && inSecBufs[1].cbBuffer) {
					size_t used = inBuffer.size() - inSecBufs[1].cbBuffer;
					ConsumeFront(inBuffer, used);
				}
				else {
					inBuffer.clear();
				}
				// Read more from server
				int n = RecvSome(s, inBuffer);
				if (n == 0) Fail("Handshake recv (connection closed)", SEC_E_INTERNAL_ERROR);
				if (n < 0) FailWSA("recv");
				phCtx = &hCtx;
				continue;
			}
			else {
				Fail("InitializeSecurityContext", ss);
			}
		}

		// Validate server certificate (hostname must match)
		VerifyServerCert(hCtx, serverName);

		// Determine negotiated protocol; ensure TLS 1.3
		SecPkgContext_ConnectionInfoEx connInfoEx{};
		ss = QueryContextAttributesW(&hCtx, SECPKG_ATTR_CONNECTION_INFO_EX, &connInfoEx);
		if (ss == SEC_E_OK) {
			negotiatedTls13 = (connInfoEx.dwProtocol == SP_PROT_TLS1_3_CLIENT);
		}
		else {
			// Fallback to older attribute if not supported (shouldn’t happen on TLS 1.3)
			SecPkgContext_ConnectionInfo ci{};
			ss = QueryContextAttributesW(&hCtx, SECPKG_ATTR_CONNECTION_INFO, &ci);
			if (ss != SEC_E_OK) Fail("QueryContextAttributes(ConnectionInfo)", ss);
			negotiatedTls13 = (ci.dwProtocol == SP_PROT_TLS1_3_CLIENT);
		}

		// Get stream sizes for EncryptMessage/DecryptMessage
		ss = QueryContextAttributesW(&hCtx, SECPKG_ATTR_STREAM_SIZES, &streamSizes);
		if (ss != SEC_E_OK) Fail("QueryContextAttributes(STREAM_SIZES)", ss);
	}

	// Optional ALPN: set to "h2" or "http/1.1" before first InitializeSecurityContext call.
	// In this example, we demonstrate HTTP/1.1 after handshake explicitly.
	int wmain(int argc, wchar_t** argv) {
		const wchar_t* serverW = L"www.google.com";
		const char* serverA = "www.google.com";
		const char* port = "443";
		if (argc >= 2) serverW = argv[1];

		WSADATA wsa{};
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) FailWSA("WSAStartup");

		SOCKET s = ConnectTcp(serverA, port);

		CredHandle hCred{};
		CtxtHandle hCtx{};
		bool isTls13 = false;
		SecPkgContext_StreamSizes sizes{};
		TlsHandshake(s, serverW, hCred, hCtx, isTls13, sizes);
		std::wcout << L"Handshake complete. TLS 1.3: " << (isTls13 ? L"yes" : L"no") << L"\n";

		// Send an HTTPS GET
		std::string httpReq = "GET / HTTP/1.1\r\nHost: ";
		httpReq += serverA;
		httpReq += "\r\nConnection: close\r\nAccept: */*\r\nUser-Agent: sspi-sample/1.0\r\n\r\n";

		// Encrypt and send
		auto cipher = SslEncrypt(hCtx, sizes, httpReq.data(), httpReq.size());
		if (SendAll(s, cipher.data(), (int)cipher.size()) <= 0) FailWSA("send(app)");

		// Read and decrypt until close or close_notify
		std::vector<char> netBuf;
		std::vector<char> plain;
		for (;;) {
			int n = RecvSome(s, netBuf);
			if (n == 0) break;      // TCP closed
			if (n < 0) FailWSA("recv");

			for (;;) {
				SECURITY_STATUS ss = SslDecrypt(hCtx, netBuf, plain);
				if (ss == SEC_E_INCOMPLETE_MESSAGE) break;     // need more ciphertext
				if (ss == SEC_I_CONTEXT_EXPIRED) {             // close_notify
					netBuf.clear();
					break;
				}
				// ss == SEC_E_OK or SEC_I_RENEGOTIATE handled; we ignore renegotiation for TLS 1.2-
				if (netBuf.empty()) break;
			}
			if (netBuf.empty() && n == 0) break;
		}

		std::cout.write(plain.data(), (std::streamsize)plain.size());
		std::cout << "\n";

		// Send close_notify
		SecBuffer closeBufs[1]{};
		SecBufferDesc closeDesc{ SECBUFFER_VERSION, 1, closeBufs };
		SECURITY_STATUS ss = ApplyControlToken(&hCtx, &closeDesc); // no-op; schannel uses shutdown via InitializeSecurityContext with ISC_REQ_ALLOCATE_MEMORY flag
		(void)ss;

		// Proper TLS shutdown with Schannel: use SCHANNEL_SHUTDOWN control token
		DWORD dwShutdown = SCHANNEL_SHUTDOWN;
		SecBuffer shutBuf{};
		shutBuf.BufferType = SECBUFFER_TOKEN;
		shutBuf.pvBuffer = &dwShutdown;
		shutBuf.cbBuffer = sizeof(dwShutdown);
		SecBufferDesc shutDesc{ SECBUFFER_VERSION, 1, &shutBuf };
		ss = ApplyControlToken(&hCtx, &shutDesc);
		if (ss == SEC_E_OK) {
			DWORD attr = 0; TimeStamp ts{};
			SecBuffer outBuf{ 0, SECBUFFER_TOKEN, nullptr };
			SecBufferDesc outDesc{ SECBUFFER_VERSION, 1, &outBuf };
			ss = InitializeSecurityContextW(
				&hCred, &hCtx, nullptr, ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM,
				0, SECURITY_NATIVE_DREP, nullptr, 0, nullptr, &outDesc, &attr, &ts);
			if (ss == SEC_E_OK && outBuf.cbBuffer && outBuf.pvBuffer) {
				SendAll(s, outBuf.pvBuffer, (int)outBuf.cbBuffer);
				FreeContextBuffer(outBuf.pvBuffer);
			}
		}

		// Cleanup
		DeleteSecurityContext(&hCtx);
		FreeCredentialsHandle(&hCred);
		shutdown(s, SD_BOTH);
		closesocket(s);
		WSACleanup();
		return 0;
	}
}
