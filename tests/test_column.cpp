#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <random>

#include "column.h"

template <typename T>
class ColumnTypedTest : public ::testing::Test {
 protected:
  using Col = Column<T>;

  T get_test_value(int index = 0) {
    if constexpr (std::is_same_v<T, int64_t>) {
      return 10 + index;
    } else if constexpr (std::is_same_v<T, double>) {
      return 1.5 * (index + 1);
    } else if constexpr (std::is_same_v<T, std::string>) {
      return "test" + std::to_string(index);
    }
  }

  T get_null_test_value() { return Utils::get_null<T>(); }
};

using MyTypes = ::testing::Types<int64_t, double, std::string>;
TYPED_TEST_SUITE(ColumnTypedTest, MyTypes);

TYPED_TEST(ColumnTypedTest, DefaultConstructor) {
  typename TestFixture::Col col;
  EXPECT_EQ(col.size(), 0);
  EXPECT_TRUE(col.empty());
  EXPECT_EQ(col.get_null_count(), 0);
}

TYPED_TEST(ColumnTypedTest, VectorConstructor) {
  int n{3};
  std::vector<TypeParam> vec{};

  for (int i{0}; i < n; ++i) {
    vec.push_back(this->get_test_value(i));
  }

  typename TestFixture::Col col(vec);

  EXPECT_EQ(col.size(), vec.size());
  EXPECT_EQ(col.get_null_count(), 0);

  for (int i{0}; i < n; ++i) {
    EXPECT_EQ(col[i], this->get_test_value(i));
  }
}

TYPED_TEST(ColumnTypedTest, VectorConstructorWithNull) {
  int n{4};
  std::vector<TypeParam> vec{};

  for (int i{0}; i < n; ++i) {
    if (i % 2 == 0) {
      vec.push_back(this->get_test_value(i));
    } else {
      vec.push_back(this->get_null_test_value());
    }
  }

  typename TestFixture::Col col(vec);

  EXPECT_EQ(col.size(), 4);
  EXPECT_EQ(col.get_null_count(), 2);

  for (int i{0}; i < n; ++i) {
    if (i % 2 == 0) {
      EXPECT_EQ(col[i], this->get_test_value(i));
    } else {
      EXPECT_TRUE(Utils::is_null(col[i]));
    }
  }
}

TYPED_TEST(ColumnTypedTest, AppendValue) {
  typename TestFixture::Col col{};

  auto value{this->get_test_value()};

  col.append(value);

  EXPECT_EQ(col.size(), 1);
  EXPECT_EQ(col.get_null_count(), 0);
  EXPECT_EQ(col[0], value);
}

TYPED_TEST(ColumnTypedTest, AppendNull) {
  typename TestFixture::Col col{};

  col.append(this->get_null_test_value());

  EXPECT_EQ(col.size(), 1);
  EXPECT_EQ(col.get_null_count(), 1);
  EXPECT_TRUE(Utils::is_null(col[0]));
}

TYPED_TEST(ColumnTypedTest, OperatorThrowsOutOfRange) {
  typename TestFixture::Col col{};
  col.append(this->get_test_value());

  EXPECT_THROW(col[5], std::out_of_range);
}

TYPED_TEST(ColumnTypedTest, NullCountManagement) {
  typename TestFixture::Col col{};

  size_t nulls{5};
  col.set_null_count(nulls);
  EXPECT_EQ(col.get_null_count(), nulls);
}

TYPED_TEST(ColumnTypedTest, MaximumCalculatesCorrectly) {
  typename TestFixture::Col col{};
  EXPECT_THROW(col.maximum(), std::invalid_argument);

  col.append(this->get_null_test_value());
  EXPECT_THROW(col.maximum(), std::invalid_argument);

  col.append(this->get_test_value());

  TypeParam max_value{};
  if constexpr (std::is_same_v<TypeParam, std::string>) {
    max_value = "zzzzz";  // lexicographical maximum
  } else {
    max_value = this->get_test_value() * 2;
  }

  col.append(max_value);
  EXPECT_EQ(col.maximum(), max_value);
}

TYPED_TEST(ColumnTypedTest, MinimumCalculatesCorrectly) {
  typename TestFixture::Col col{};
  EXPECT_THROW(col.minimum(), std::invalid_argument);

  col.append(this->get_null_test_value());
  EXPECT_THROW(col.minimum(), std::invalid_argument);

  col.append(this->get_test_value());
  TypeParam min_value{};
  if constexpr (std::is_same_v<TypeParam, std::string>) {
    min_value = "aaaaa";  // lexicographical minimum
  } else {
    min_value = this->get_test_value() - 5;
  }

  col.append(min_value);
  EXPECT_EQ(col.minimum(), min_value);
}

TYPED_TEST(ColumnTypedTest, ModeCalculatesCorrectly) {
  typename TestFixture::Col col{};
  EXPECT_THROW(col.mode(), std::invalid_argument);

  col.append(this->get_null_test_value());
  EXPECT_THROW(col.mode(), std::invalid_argument);

  // handles no modes
  int n{5};
  for (int i{0}; i < n; ++i) {
    col.append(this->get_test_value(i));
  }
  auto no_modes{col.mode()};
  EXPECT_TRUE(no_modes.empty());

  TypeParam first_mode{this->get_test_value()};
  TypeParam second_mode{this->get_test_value(1)};

  // handles 1 mode
  col.append(first_mode);
  auto one_mode{col.mode()};
  EXPECT_FALSE(one_mode.empty());
  EXPECT_EQ(one_mode.size(), 1);
  EXPECT_THAT(one_mode, testing::ElementsAre(first_mode));

  // handles >1 modes
  col.append(second_mode);
  auto two_modes{col.mode()};
  EXPECT_FALSE(two_modes.empty());
  EXPECT_EQ(two_modes.size(), 2);
  EXPECT_THAT(two_modes,
              testing::UnorderedElementsAre(first_mode, second_mode));
}

TYPED_TEST(ColumnTypedTest, SumCalculatesCorrectly) {
  typename TestFixture::Col col{};
  if constexpr (std::is_same_v<TypeParam, std::string>) {
    EXPECT_THROW(col.sum(), std::invalid_argument);
  } else {
    EXPECT_THROW(col.sum(), std::invalid_argument);
    col.append(this->get_null_test_value());
    EXPECT_THROW(col.sum(), std::invalid_argument);

    int n{3};
    for (int i{0}; i < n; ++i) {
      col.append(this->get_test_value());
    }

    EXPECT_EQ(col.sum(), static_cast<double>(this->get_test_value() * n));
  }
}

TYPED_TEST(ColumnTypedTest, MedianCalculatesCorrectly) {
  typename TestFixture::Col col{};
  if constexpr (std::is_same_v<TypeParam, std::string>) {
    EXPECT_THROW(col.median(), std::invalid_argument);
  } else {
    EXPECT_THROW(col.median(), std::invalid_argument);
    col.append(this->get_null_test_value());
    EXPECT_THROW(col.median(), std::invalid_argument);

    std::vector<TypeParam> odd_length{1, 2, 3, 4, 5, 6, 7};
    double expected_median_for_odd_length{4.0};

    std::vector<TypeParam> even_length{1, 2, 3, 4, 5, 6};
    double expected_median_for_even_length{3.5};

    std::mt19937 gen(12345);
    std::shuffle(odd_length.begin(), odd_length.end(), gen);
    std::shuffle(even_length.begin(), even_length.end(), gen);

    for (const auto& value : odd_length) {
      col.append(value);
    }
    EXPECT_EQ(col.median(), expected_median_for_odd_length);

    col.clear();

    for (const auto& value : even_length) {
      col.append(value);
    }
    EXPECT_EQ(col.median(), expected_median_for_even_length);
  }
}

TYPED_TEST(ColumnTypedTest, MeanCalculatesCorrectly) {
  typename TestFixture::Col col{};
  if constexpr (std::is_same_v<TypeParam, std::string>) {
    EXPECT_THROW(col.mean(), std::invalid_argument);
  } else {
    EXPECT_THROW(col.mean(), std::invalid_argument);
    col.append(this->get_null_test_value());
    EXPECT_THROW(col.mean(), std::invalid_argument);

    std::vector<TypeParam> positive_only{1, 2, 3, 4, 5};
    double expected_mean_for_positives_only{3.0};

    std::vector<TypeParam> with_negatives{-5, -4, 3, 2};
    double expected_mean_for_with_negatives{-1.0};

    for (const auto& value : positive_only) {
      col.append(value);
    }
    EXPECT_EQ(col.mean(), expected_mean_for_positives_only);

    col.clear();

    for (const auto& value : with_negatives) {
      col.append(value);
    }
    EXPECT_EQ(col.mean(), expected_mean_for_with_negatives);
  }
}

TYPED_TEST(ColumnTypedTest, StandardDeviationCalculatesCorrectly) {
  typename TestFixture::Col col{};
  if constexpr (std::is_same_v<TypeParam, std::string>) {
    EXPECT_THROW(col.standard_deviation(), std::invalid_argument);
  } else {
    EXPECT_THROW(col.standard_deviation(), std::invalid_argument);
    col.append(this->get_null_test_value());
    EXPECT_THROW(col.standard_deviation(), std::invalid_argument);

    std::vector<TypeParam> values{2, 4, 6, 8, 10};
    double expected_std{std::sqrt(10.0)};

    for (const auto& value : values) {
      col.append(value);
    }
    EXPECT_EQ(col.standard_deviation(), expected_std);
  }
}

TYPED_TEST(ColumnTypedTest, VarianceCalculatesCorrectly) {
  typename TestFixture::Col col{};
  if constexpr (std::is_same_v<TypeParam, std::string>) {
    EXPECT_THROW(col.variance(), std::invalid_argument);
  } else {
    EXPECT_THROW(col.variance(), std::invalid_argument);
    col.append(this->get_null_test_value());
    EXPECT_THROW(col.variance(), std::invalid_argument);

    std::vector<TypeParam> values{2, 4, 6, 8, 10};
    double expected_var{10.0};

    for (const auto& value : values) {
      col.append(value);
    }
    EXPECT_EQ(col.variance(), expected_var);
  }
}