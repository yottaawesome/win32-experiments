export module progressbar:status;
import std;

export namespace SimpleProgressWindow
{
	struct ProgressStatus
	{
		std::wstring Title;
		std::wstring Text;

		int Value = 0;
		int Min = 0;
		int Max = 100;

		bool Marquee = false;
	};
}
