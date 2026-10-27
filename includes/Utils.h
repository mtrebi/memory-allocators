#ifndef UTILS_H
#define UTILS_H

// D-38: 原实现缺 <cstddef>，头文件不自包含，单独包含即编译失败。
#include <cstddef>

class Utils {
public:
	// D-06: 原实现用 (baseAddress / alignment) + 1 计算下一个对齐点，
	// 当地址本身已对齐时仍会再加一个 alignment，凭空浪费 alignment 字节，
	// 并且会让 CalculatePaddingWithHeader 的返回值为 alignment 而非 headerSize。
	static std::size_t CalculatePadding(const std::size_t baseAddress, const std::size_t alignment) {
		if (alignment == 0) return 0;
		const std::size_t remainder = baseAddress % alignment;
		return remainder == 0 ? 0 : alignment - remainder;
	}

	static std::size_t CalculatePaddingWithHeader(const std::size_t baseAddress, const std::size_t alignment, const std::size_t headerSize) {
		// D-25: alignment == 0 时下面的 "% alignment" 会除零。
		// 语义上视作"不需要额外对齐"，只预留块头空间。
		if (alignment == 0) return headerSize;

		std::size_t padding = CalculatePadding(baseAddress, alignment);
		std::size_t neededSpace = headerSize;

		if (padding < neededSpace){
			// Header does not fit - Calculate next aligned address that header fits
			neededSpace -= padding;

			// How many alignments I need to fit the header        
        	if(neededSpace % alignment > 0){
		        padding += alignment * (1+(neededSpace / alignment));
        	}else {
		        padding += alignment * (neededSpace / alignment);
        	}
		}

		return padding;
	}
};

#endif /* UTILS_H */
