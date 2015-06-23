#include <iostream>
#include <complex>
#include "../imtoolbox.h"
using namespace imtoolbox;
using namespace std;

int main() {
  /*
  auto mat0 = avg_folder<double>("n2.1/sample/", "avg1.png", 5);
  auto mat01 = imread<uint8_t>("avg1.png");
  auto mat02 = imread<uint8_t>("avg.png");              
  assert(mat01 == mat02);
  print_i("PASS!\n");
  */

  matrix2<int> mat = {
      {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};

  assert(sum(mat, 0) == matrix1<int>({28, 32, 36, 40}));
  assert(sum(mat, 1) == matrix1<int>({10, 26, 42, 58}));

  auto mat1 = mat(slice{0, 1}, slice{0, 1});
  assert(sum(mat1, 0) == matrix1<int>({6, 8}));
  assert(sum(mat1, 1) == matrix1<int>({3, 11}));
  assert(sum(sum(mat1)) == 14);

  matrix1<int> mat2(0);
  assert(sum(mat2, 0) == 0);

  println_i("PASS!");

  matrix2<int> h = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

  matrix2<int> h1 = {{1, 2}, {3, 4}};

  matrix2<int> m = {
      {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};

  // Test filter
  assert(filter2(h, m, filter2_t::valid) ==
         matrix2<int>({{348, 393}, {528, 573}}));
  assert((filter2(m, h, filter2_t::valid) == empty_matrix<int, 2>()));
  assert(filter2(h1, m, filter2_t::valid) ==
         matrix2<int>({{44, 54, 64}, {84, 94, 104}, {124, 134, 144}}));
  assert((filter2(m, h1, filter2_t::valid) == empty_matrix<int, 2>()));

  assert(filter2(h, m, filter2_t::full) ==
         matrix2<int>({{9, 26, 50, 74, 53, 28},
                       {51, 111, 178, 217, 145, 72},
                       {114, 231, 348, 393, 252, 120},
                       {186, 363, 528, 573, 360, 168},
                       {105, 197, 274, 295, 175, 76},
                       {39, 68, 86, 92, 47, 16}}));
  assert(filter2(m, h, filter2_t::full) ==
         matrix2<int>({{16, 47, 92, 86, 68, 39},
                       {76, 175, 295, 274, 197, 105},
                       {168, 360, 573, 528, 363, 186},
                       {120, 252, 393, 348, 231, 114},
                       {72, 145, 217, 178, 111, 51},
                       {28, 53, 74, 50, 26, 9}}));
  assert(filter2(h1, m, filter2_t::full) ==
         matrix2<int>({{4, 11, 18, 25, 12},
                       {22, 44, 54, 64, 28},
                       {46, 84, 94, 104, 44},
                       {70, 124, 134, 144, 60},
                       {26, 41, 44, 47, 16}}));

  assert(filter2(m, h1, filter2_t::full) ==
         matrix2<int>({{16, 47, 44, 41, 26},
                       {60, 144, 134, 124, 70},
                       {44, 104, 94, 84, 46},
                       {28, 64, 54, 44, 22},
                       {12, 25, 18, 11, 4}}));

  assert(filter2(h, m, filter2_t::same) ==
         matrix2<int>({{111, 178, 217, 145},
                       {231, 348, 393, 252},
                       {363, 528, 573, 360},
                       {197, 274, 295, 175}}));

  assert(filter2(m, h, filter2_t::same) ==
         matrix2<int>({{573, 528, 363}, {393, 348, 231}, {217, 178, 111}}));

  assert(filter2(h1, m, filter2_t::same) == matrix2<int>({{44, 54, 64, 28},
                                                          {84, 94, 104, 44},
                                                          {124, 134, 144, 60},
                                                          {41, 44, 47, 16}}));

  assert(filter2(m, h1, filter2_t::same) == matrix2<int>({{94, 84}, {54, 44}}));
  print_i("PASS!\n");

  std::vector<double> v = {25, 8, 15, 5, 6, 10,
                           10 + std::numeric_limits<double>::epsilon(), 3, 1,
                           20, 7};
  auto pks = findpeaks(v.begin(), v.end());
  for (size_t i = 0; i < pks.size(); ++i) {
    println_i(pks[i].first, ", ", pks[i].second + 1);
  }

  matrix2<int> x, y;
  std::tie(x, y) = meshgrid(g_slice<int>{-1, 1}, g_slice<int>{-2, 2});
  assert(x == matrix2<int>({{-1,0,1},{-1,0,1},{-1,0,1},{-1,0,1},{-1,0,1}}));
  assert(y == matrix2<int>({{-2,-2,-2},{-1,-1,-1},{0,0,0},{1,1,1},{2,2,2}}));
  println_i("PASS!");


  println_i(fspecial_gaussian<double>(std::make_pair(3, 3), 0.5));
  //println_i(fspecial<double>(fspecial_t::gaussian, std::make_pair(4, 4), 0.5));
  //println_i(fspecial<double>(fspecial_t::average, std::make_pair(4, 4));
  return 0;
}
