#pragma once

#include <cstdint>
#include <vector>

#include "ItemProcessRequest.hpp"

template <typename T>
class ItemRegistry
{
public:
	uint32_t add(T&& item);
	std::vector<uint32_t> add(std::vector<T>&& items);
	const T& get(uint32_t slot);
	const std::vector<T>& getItems();
private:
	uint32_t next = 0;
	std::vector<T> items;
};

template<typename T>
uint32_t ItemRegistry<T>::add(T&& item)
{
	uint32_t slot = next++;
	items.push_back(std::move(item));
	return slot;
}

template<typename T>
std::vector<uint32_t> ItemRegistry<T>::add(std::vector<T>&& items)
{
	std::vector<uint32_t> slots;
	for (T& item : items)
	{
		slots.push_back(add(std::move(item)));
	}
	return slots;
}

template<typename T>
const T& ItemRegistry<T>::get(uint32_t slot)
{
	return items[slot];
}

template<typename T>
const std::vector<T>& getItems()
{
	return items;
}

struct RegistryCollection
{
	ItemRegistry<ImageProcessRequest> imageRegistry;
	ItemRegistry<SamplerProcessRequest> samplerRegistry;
	ItemRegistry<TextureProcessRequest> textureRegistry;
	ItemRegistry<MaterialProcessRequest> materialRegistry;
};