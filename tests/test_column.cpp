#include <gtest/gtest.h>

#include "column.h"

template <typename T>
class ColumnTypedTest : public ::testing::Test {
 protected:
  using Col = Column<T>;

  ColumnType get_test_type() {
    if constexpr (std::is_same_v<T, int64_t>) {
      return ColumnType::Int64;
    } else if constexpr (std::is_same_v<T, double>) {
      return ColumnType::Double;
    } else if constexpr (std::is_same_v<T, std::string>) {
      return ColumnType::String;
    }
  }

  T get_test_value(int index) {
    if constexpr (std::is_same_v<T, int64_t>) {
      return 10 + index;
    } else if constexpr (std::is_same_v<T, double>) {
      return 1.5 * (index + 1);
    } else if constexpr (std::is_same_v<T, std::string>) {
      return "test" + std::to_string(index);
    }
  }

  T get_null_test_value() {
    if constexpr (std::is_same_v<T, int64_t>) {
      return std::numeric_limits<int64_t>::min();
    } else if constexpr (std::is_same_v<T, double>) {
      return std::numeric_limits<double>::quiet_NaN();
    } else if constexpr (std::is_same_v<T, std::string>) {
      return "";
    }
  }
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

  typename TestFixture::Col col(vec, this->get_test_type());

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

  typename TestFixture::Col col(vec, this->get_test_type());

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

  auto value{this->get_test_value(0)};

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
  col.append(this->get_test_value(0));

  EXPECT_THROW(col[5], std::out_of_range);
}

TYPED_TEST(ColumnTypedTest, NullCountManagement) {
  typename TestFixture::Col col{};

  size_t nulls{5};
  col.set_null_count(nulls);
  EXPECT_EQ(col.get_null_count(), nulls);
}