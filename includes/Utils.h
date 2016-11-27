#ifndef UTILS_H
#define UTILS_H

class Utils {
public:
	static const std::size_t CalculatePadding(const std::size_t baseAddress, const std::size_t alignment) {
		const std::size_t multiplier = (baseAddress / alignment) + 1;
		const std::size_t alignedAddress = multiplier * alignment;
		const std::size_t padding = alignedAddress - baseAddress;
		return padding;
	}
};

#endif /* UTILS_H */