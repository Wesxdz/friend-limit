#include <unordered_map>
#include <typeindex>
#include "Utils/id_pool.h"

class Pool
{
    public:
        // PropTypeId
        static inline std::unordered_map<size_t, IdPool<size_t>> ids;

        template <typename T>
        static constexpr size_t GetPropTypeId()
        {
            return std::type_index(typeid(std::decay_t<T>)).hash_code();
        }
};
