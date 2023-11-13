import demo;

int main(int argc, char* argv[])
{
    if (argc > 1)
        return Demo::VerifyEmbeddedSignature::Run((LPCWSTR)argv[1]);
    //return Demo::AuthenticodeSigned::Run(argc, argv);

    return 0;
}