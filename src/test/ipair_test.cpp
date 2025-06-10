#include "shared/assert_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/ipair.h"

#include <array>
#include <memory>
#include <vector>

// Naming scheme: ipair
// Contains tests that use the ktl::ipair

namespace ktl::test::ipair
{
    struct big
    {
        double a;
        double b;
    };

    template<typename C>
    class non_const_iterable
    {
    public:
        using iterator = decltype(std::declval<C&>().begin());

    public:
        template<typename V>
        non_const_iterable(V&& container) :
            m_Container(std::forward<V>(container)) {}

        iterator begin() { return m_Container.begin(); }

        iterator end() { return m_Container.end(); }

    private:
        C m_Container;
    };

    template<typename C>
    class only_const_iterable
    {
    public:
        using const_iterator = decltype(std::declval<C&>().begin());

    public:
        template<typename V>
        only_const_iterable(V&& container) :
            m_Container(std::forward<V>(container)) {}

        const_iterator begin() const { return m_Container.begin(); }

        const_iterator end() const { return m_Container.end(); }

    private:
        C m_Container;
    };

    KTL_ADD_TEST(test_ipair_full)
    {
        std::vector<big> vec;
        vec.emplace_back(big{ 2.4, 5.3 });
        vec.emplace_back(big{ 1.0, 0.0 });

        ipair_iterable<size_t, std::vector<big>&> pair = ktl::ipair(vec);

        ipair_iterator<size_t, std::vector<big>::iterator> first = pair.begin();
        ipair_iterable<size_t, std::vector<big>&>::iterator last = pair.end();

        ipair_value<size_t, big&> element = *first;
    }

    KTL_ADD_TEST(test_ipair_reference)
    {
        std::vector<big> vec;
        vec.emplace_back(big{ 2.4, 5.3 });
        vec.emplace_back(big{ 1.0, 0.0 });
        vec.emplace_back(big{ -1.0, -4.0 });

        auto pair = ktl::ipair(vec);

        {
            size_t i = 0;
            for (auto kv : pair)
            {
                KTL_TEST_ASSERT(i == kv.first);
                KTL_TEST_ASSERT(vec[i].a == kv.second.a);
                ++i;
            }
        }

        {
            size_t i = 0;
            for (auto [index, value] : pair)
            {
                KTL_TEST_ASSERT(i == index);
                KTL_TEST_ASSERT(vec[i].a == value.a);
                ++i;
            }
        }
    }

    KTL_ADD_TEST(test_const_ipair)
    {
        std::vector<big> vec;
        vec.emplace_back(big{ 2.4, 5.3 });
        vec.emplace_back(big{ 1.0, 0.0 });

        const auto& const_vec = vec;

        const auto pair = ktl::ipair(const_vec);

        auto first = pair.begin();
        auto last = pair.end();
    }

    KTL_ADD_TEST(test_only_const_ipair)
    {
        std::vector<big> vec;
        vec.emplace_back(big{ 2.4, 5.3 });
        vec.emplace_back(big{ 1.0, 0.0 });

        auto const_vec = only_const_iterable<const std::vector<big>&>(vec);

        auto pair = ktl::ipair(const_vec);

        auto first = pair.begin();
        auto last = pair.end();
    }

    KTL_ADD_TEST(test_non_const_ipair)
    {
        std::vector<big> vec;
        vec.emplace_back(big{ 2.4, 5.3 });
        vec.emplace_back(big{ 1.0, 0.0 });

        auto non_const_vec = non_const_iterable<std::vector<big>&>(vec);

        auto pair = ktl::ipair(non_const_vec);

        auto first = pair.begin();
        auto last = pair.end();
    }

    KTL_ADD_TEST(test_ipair_inline)
    {
        auto pair = ktl::ipair(std::vector<big>({ big{ 0.0, 1.0 } }));

        auto first = pair.begin();
        auto last = pair.end();
    }

    KTL_ADD_TEST(test_ipair_inline_array)
    {
        auto pair = ktl::ipair(std::array<big, 1>{{ 0.0, 1.0 }});

        auto first = pair.begin();
        auto last = pair.end();
    }
}