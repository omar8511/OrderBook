#include "orderedpricelevel.hpp"
#include <algorithm>
#include <stdexcept>

// Iterator implementations
std::pair<uint32_t, PriceLevel>& OrderedPriceLevel::iterator::operator*() const {
	if(currentIndex >= levels->size()){
		throw std::out_of_range("Iterator out of bounds");
	}
	return (*levels)[currentIndex];
}

std::pair<uint32_t, PriceLevel>* OrderedPriceLevel::iterator::operator->() const {
	if(currentIndex >= levels->size()){
		throw std::out_of_range("Iterator out of bounds");
	}
	return &((*levels)[currentIndex]);
}

OrderedPriceLevel::iterator& OrderedPriceLevel::iterator::operator++(){
	++currentIndex;
	return *this;
}

OrderedPriceLevel::iterator OrderedPriceLevel::iterator::operator++(int){
	iterator temp = *this;
	++currentIndex;
	return temp;
}

OrderedPriceLevel::iterator& OrderedPriceLevel::iterator::operator--(){
	--currentIndex;
	return *this;
}

OrderedPriceLevel::iterator OrderedPriceLevel::iterator::operator--(int){
	iterator temp = *this;
	--currentIndex;
	return temp;
}

bool OrderedPriceLevel::iterator::operator==(const iterator& other) const {
	return currentIndex == other.currentIndex && levels == other.levels;
}

bool OrderedPriceLevel::iterator::operator!=(const iterator& other) const {
	return !(*this == other);
}

// OrderedPriceLevel implementations
OrderedPriceLevel::iterator OrderedPriceLevel::find(uint32_t price){
	auto it = priceToIndex.find(price);
	if(it == priceToIndex.end()) return end();
	size_t elementIndex = it->second;
	return iterator(&sortedLevels, elementIndex);
}

OrderedPriceLevel::iterator OrderedPriceLevel::begin(){
	return iterator(&sortedLevels, 0);
}

OrderedPriceLevel::iterator OrderedPriceLevel::end(){
	size_t size = sortedLevels.size();
	return iterator(&sortedLevels, size);
}

OrderedPriceLevel::reverse_iterator OrderedPriceLevel::rbegin(){
	return reverse_iterator(end());
}

OrderedPriceLevel::reverse_iterator OrderedPriceLevel::rend(){
	return reverse_iterator(begin());
}

OrderedPriceLevel::iterator OrderedPriceLevel::erase(iterator it){
 	size_t index = it.getCurrentIndex();
    uint32_t price = (*it).first; 
    sortedLevels.erase(sortedLevels.begin() + index);

    priceToIndex.erase(price);

    // Update indices in hash map
    for (size_t i = index; i < sortedLevels.size(); ++i) {
        priceToIndex[sortedLevels[i].first] = i;
    }

    if (index >= sortedLevels.size()) {
        return end();
    }
    return iterator(&sortedLevels, index);
}

std::pair<OrderedPriceLevel::iterator, bool> OrderedPriceLevel::emplace(uint32_t price, PriceLevel&& level) {
    auto mapIt = priceToIndex.find(price);
    if (mapIt != priceToIndex.end()) {
        return { iterator(&sortedLevels, mapIt->second), false };
    }

    auto comp = [](const std::pair<uint32_t, PriceLevel>& elem, uint32_t val) {
        return elem.first < val;
    };
    auto vecIt = std::lower_bound(sortedLevels.begin(), sortedLevels.end(), price, comp);

    size_t insertIndex = std::distance(sortedLevels.begin(), vecIt);

    sortedLevels.insert(vecIt, std::make_pair(price, std::move(level)));

    // Update indices in hash map
    for (size_t i = insertIndex; i < sortedLevels.size(); ++i) {
        priceToIndex[sortedLevels[i].first] = i;
    }

    return { iterator(&sortedLevels, insertIndex), true };
}