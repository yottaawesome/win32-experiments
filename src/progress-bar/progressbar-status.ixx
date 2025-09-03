export module progressbar:status;
import std;

export namespace SimpleProgressWindow
{
	struct ProgressStatus
	{
		std::wstring Title;
		std::wstring Text;

		int Value;
		int Min;
		int Max;

		bool Marquee;

		ProgressStatus(std::wstring title, std::wstring text, int value = 0, int min = 0, int max = 100, bool marquee = 0)
		{
			Title = title;
			Text = text;
			Value = value;
			Min = min;
			Max = Max;
			Marquee = marquee;
		}
	};
}
