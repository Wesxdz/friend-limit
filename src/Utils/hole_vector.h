#pragma once

#include "id_pool.h"

template<typename T, typename Id = size_t>
class HoleVector
{
public:
	struct Hole
	{
		bool fill = false;
		T object = {};
	};
    
	class iterator
    {
        public:
            typedef iterator self_type;
            typedef T value_type;
            typedef T& reference;
            typedef T* pointer;
            typedef std::forward_iterator_tag iterator_category;
            typedef int difference_type;
            iterator(Hole* ptr) : ptr(ptr) { }
            self_type operator++(){ self_type i = *this; advance(); return i; }
            self_type operator++(int junk) { advance(); return *this; }
            reference operator*() { return ptr->object; }
            pointer operator->() { return &ptr->object; }
            bool operator==(const self_type& rhs) { return ptr == rhs.ptr; }
            bool operator!=(const self_type& rhs) { return ptr != rhs.ptr; }

            void advance()
            {
            	while(!(++ptr)->fill);
            }
        private:
            Hole* ptr;
    };

	class const_iterator
    {
        public:
            typedef const_iterator self_type;
            typedef T value_type;
            typedef T& reference;
            typedef T* pointer;
            typedef const T& reference_const;
            typedef const T* pointer_const;
            typedef std::forward_iterator_tag iterator_category;
            typedef int difference_type;
            const_iterator(Hole* ptr) : ptr(ptr) { }
            self_type operator++(){ self_type i = *this; advance(); return i; }
            self_type operator++(int junk) { advance(); return *this; }
            reference_const operator*() { return ptr->object; }
            pointer_const operator->() { return &ptr->object; }
            bool operator==(const self_type& rhs) { return ptr == rhs.ptr; }
            bool operator!=(const self_type& rhs) { return ptr != rhs.ptr; }

            void advance()
            {
            	while(!(++ptr)->fill);
            }
        private:
            Hole* ptr;
    };

    iterator begin()
    {
        return iterator(buffer.data());
    }

    iterator end()
    {
        return iterator(buffer.data() + buffer.size() - 1);
    }

    const_iterator begin() const
    {
        return const_iterator(buffer.data());
    }

    const_iterator end() const
    {
        return const_iterator(buffer.data() + buffer.size() - 1);
    }

	std::pair<Id, T&> next()
	{
		const Id id = idPool.next();

		if(id >= (buffer.size() - 1))
		{
            buffer.resize(id + 2);
            buffer.back().fill = true;
		}

        Hole& hole = buffer[id];
		hole.fill = true;

		return {id, hole.object};
	}

	void free(T id)
	{
		buffer[id] = {};
		idPool.free(id);
	}

	HoleVector()
	{
		buffer.resize(1);
		buffer[0].fill = true;
	}

	T& operator[](Id id)
	{
		return buffer[id].object;
	}

	const T& operator[](Id id) const
	{
		return buffer[id].object;
	}

//private:
	std::vector<Hole> buffer;
	IdPool<Id, true> idPool;
};