#ifndef ORDEREDPRICELEVEL_HPP
#define ORDEREDPRICELEVEL_HPP

#include <vector>
#include <utility>
#include <iterator>
#include <unordered_map>
#include "pricelevel.hpp"

class OrderedPriceLevel{
public: 
	struct iterator {
		iterator(std::vector<std::pair<uint32_t, PriceLevel>> *_levels, size_t _currentIndex) : levels(_levels), currentIndex(_currentIndex) {};
		
		// Iterator traits
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = std::pair<uint32_t, PriceLevel>;
		using difference_type = std::ptrdiff_t;
		using pointer = std::pair<uint32_t, PriceLevel>*;
		using reference = std::pair<uint32_t, PriceLevel>&;
		
		// Operators
		std::pair<uint32_t, PriceLevel>& operator*() const;
		std::pair<uint32_t, PriceLevel>* operator->() const;
		iterator& operator++();
		iterator operator++(int);
		iterator& operator--();
		iterator operator--(int);
		bool operator==(const iterator& other) const;
		bool operator!=(const iterator& other) const;
		
		
		size_t getCurrentIndex() const { return currentIndex; }
		
	private:
		std::vector<std::pair<uint32_t, PriceLevel>> *levels;
		size_t currentIndex;
	};
	
	using reverse_iterator = std::reverse_iterator<iterator>;
	
	iterator find(uint32_t price);
	iterator begin();		
	iterator end();
	reverse_iterator rbegin();
	reverse_iterator rend();
	
	iterator erase(iterator it);
	std::pair<iterator, bool> emplace(uint32_t price, PriceLevel&& level);
	
private:
	std::vector<std::pair<uint32_t, PriceLevel>> sortedLevels;
	std::unordered_map<uint32_t, size_t> priceToIndex;
};

#endif
