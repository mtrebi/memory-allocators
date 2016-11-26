#ifndef UTILS_H
#define UTILS_H

class Utils {
public:
	static const std::size_t CalculatePadding(const std::size_t offset, const std::size_t alignment) {
		const std::size_t multiplier = (offset / alignment) + 1;
		const std::size_t padding = (multiplier * alignment) - offset;
		return padding;
	}

};

#endif /* UTILS_H */