#include "Dynamic.h"
#include "Clipboard.h"
#include <Cocoa/Cocoa.h>
#include <iostream>

namespace Clipboard
{
	static int changeCount = -1;

	class CocoaClipboardImpl : public ClipboardImpl
	{
	public:
		void SetClipboardData() final override
		{
			@autoreleasepool
			{
				NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
				NSString *format = [NSString stringWithUTF8String:clipboardFormatName.c_str()];
				changeCount = [pasteboard declareTypes:[NSArray arrayWithObject:format] owner:nil];
				std::vector<char> saveData;
				SerializeClipboard(saveData);
				const auto *base = &saveData[0];
				auto size = saveData.size();
				NSData *data = [NSData dataWithBytes:base length:size];
				if (![pasteboard setData:data forType:format])
				{
					std::cerr << "cannot put save on clipboard: [pasteboard setData] failed" << std::endl;
					return;
				}
			}
			std::cerr << "put save on clipboard" << std::endl;
		}

		GetClipboardDataResult GetClipboardData() final override
		{
			GetClipboardDataChanged gdc;
			@autoreleasepool
			{
				NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
				int newChangeCount = [pasteboard changeCount];
				if (changeCount == newChangeCount)
				{
					return GetClipboardDataUnchanged{};
				}
				changeCount = newChangeCount;
				NSString *format = [NSString stringWithUTF8String:clipboardFormatName.c_str()];
				NSString *available = [pasteboard availableTypeFromArray:[NSArray arrayWithObject:format]];
				if (![available isEqualToString:format])
				{
					std::cerr << "not getting save from clipboard: no data" << std::endl;
					return GetClipboardDataFailed{};
				}
				NSData *data = [pasteboard dataForType:format];
				if (data == nil)
				{
					std::cerr << "not getting save from clipboard: [pasteboard dataForType] failed" << std::endl;
					return GetClipboardDataFailed{};
				}
				auto *base = reinterpret_cast<const char *>([data bytes]);
				auto size = [data length];
				gdc.data = std::vector<char>(base, base + size);
			}
			return gdc;
		}
	};

	std::unique_ptr<ClipboardImpl> CocoaClipboardFactory()
	{
		return std::make_unique<CocoaClipboardImpl>();
	}
}
