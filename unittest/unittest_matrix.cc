#include <iostream>
#include <cassert>
#include <cstdint>
#include "../imtoolbox.h"
using namespace std;
using namespace imtoolbox;

int main() {
  // Test 1:
  // Purpose:
  //    - Ability of creating matrix from list
  //    - Random access using ()
  matrix2<int> m0 = {
      {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
  assert(m0(0, 0) == 1 && m0(m0.rows() - 1, m0.cols() - 1) == 16 &&
         m0(1, 3) == 8 && m0(2, 1) == 10);
  auto m1 = m0(slice{0, 2}, slice{0, 2});
  matrix2<int> m1_truth = {{1, 2, 3}, {5, 6, 7}, {9, 10, 11}};
  assert(m1 == m1_truth);
  print_i("1: PASS!\n");

  // Test 2:
  // Purpose:
  //    - Ability of slicing of matrix_ref using start and end
  //    - Random access using ()
  auto m2 = m1(slice{1, 2}, slice{1, 2});
  assert(m2(0, 0) == 6 && m2(0, 1) == 7 && m2(1, 0) == 10 && m2(1, 1) == 11);
  matrix2<int> m2_truth = {{6, 7}, {10, 11}};
  assert(m2 == m2_truth);
  print_i("2: PASS!\n");

  // Test 3:
  // Purpose:
  //    - Ability of slicing of matrix using start and end as well as slice::all
  auto m3 = m0(slice{0, 2}, slice::all);
  matrix2<int> m3_truth = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};
  assert(m3 == m3_truth);
  print_i("3: PASS!\n");

  // Test 4:
  // Purpose:
  //    - Ability of slicing using just start
  auto m4 = m0(slice{0}, slice{1});
  matrix2<int> m4_truth = {{2, 3, 4}, {6, 7, 8}, {10, 11, 12}, {14, 15, 16}};
  assert(m4 == m4_truth);
  print_i("4: PASS!\n");

  // Test 5:
  // Purpose:
  //    - Ability of slicing using mixed integer and slice
  auto m5 = m0(0, slice{0, 2});
  matrix<int, 2> m5_truth = {{1, 2, 3}};
  assert(m5 == m5_truth);
  print_i("5: PASS!\n");

  // Test 6:
  // Purpose:
  //    - Ability of cloning
  auto m6 = m0.clone();
  m6(0, 0) = 69;
  matrix2<int> m6_truth = {
      {69, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
  assert(m6 == m6_truth);
  assert(m6 != m0);
  print_i("6: PASS!\n");

  // Test 7:
  // Purpose:
  //    - Ability of slicing using > 1 stride
  //    - Ability of assigning list to matrix
  //    - Ability of assigning matrix_ref to matrix
  auto m7 = m0(slice(0, 2, 2), slice(0, 2, 2));
  matrix2<int> m7_truth = {{1, 3}, {9, 11}};
  assert(m7 == m7_truth);
  auto m7_1 = m0(slice(1), slice(1));
  m7 = m7_1;
  m7_truth = {{6, 7, 8}, {10, 11, 12}, {14, 15, 16}};
  assert(m7 == m7_truth);
  auto m7_2(m7_1);
  assert(m7_2 == m7_truth);
  print_i("7: PASS!\n");

  // Test 8:
  // Purpose:
  //    - Ability of matrix using +=, -=, *=, /=, ^= with scalar
  auto m8 = m0.clone();
  ((((m8 += 5) -= 5) *= 2) /= 2) ^= 2;
  matrix2<int> m8_truth = {{1, 4, 9, 16},
                           {25, 36, 49, 64},
                           {81, 100, 121, 144},
                           {169, 196, 225, 256}};
  assert(m8 == m8_truth);
  print_i("8: PASS!\n");

  // Test 9:
  // Purpose:
  //    - Ability of matrix using ^= with 0
  matrix2<int> m9 = {
      {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
  m9 ^= 0;
  matrix2<int> m9_truth = {
      {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};
  assert(m9 == m9_truth);
  print_i("9: PASS!\n");

  // Test 10:
  // Purpose:
  //    - Ability of matrix_ref using +=, -=. *=, /=, ^= with scalar
  auto m10 = m0.clone();
  auto m10_1 = m10(slice{0, 2}, slice{0, 2});
  ((((m10_1 += 5) -= 5) *= 2) /= 2) ^= 2;
  matrix2<int> m10_truth = {{1, 4, 9}, {25, 36, 49}, {81, 100, 121}};
  assert(m10_1 == m10_truth);
  m10_truth = {{1, 4, 9, 4}, {25, 36, 49, 8}, {81, 100, 121, 12}, {13, 14, 15, 16}};
  assert(m10 == m10_truth);
  print_i("10: PASS!\n");

  // Test 11:
  // Purpose:
  //    - Ability of matrix using ^= with 0
  auto m11 = m10(slice{0, 2}, slice{0, 1});
  m11 ^= 0;
  matrix2<int> m11_truth = {{1, 1}, {1, 1}, {1, 1}};
  assert(m11 == m11_truth);
  print_i("11: PASS!\n");

  // Test 12:
  // Purpose:
  //    - Ability of matrix using += with another matrix
  matrix2<int> m12 = {{1, 2}, {3, 4}};
  auto m12_1 = m0(slice{0, 1}, slice{0, 1});
  m12 += m12_1;
  matrix2<int> m12_truth = {{2, 4}, {8, 10}};
  assert(m12 == m12_truth);
  print_i("12: PASS!\n");

  // Test 13:
  // Purpose:
  //    - Ability of matrix using -= with another matrix
  matrix2<int> m13 = {{1, 2}, {3, 4}};
  auto m13_1 = m0(slice{0, 1}, slice{0, 1});
  m13 -= m13_1;
  matrix2<int> m13_truth = {{0, 0}, {-2, -2}};
  assert(m13 == m13_truth);
  print_i("13: PASS!\n");

  // Test 14:
  // Purpose:
  //    - Ability of matrix_ref using += with another matrix
  matrix2<int> m14 = {{1, 2}, {3, 4}};
  auto m14_1 = m0(slice{0, 1}, slice{0, 1});
  m14_1 += m14;
  matrix2<int> m14_truth = {{2, 4}, {8, 10}};
  assert(m14_1 == m14_truth);
  print_i("14: PASS!\n");

  // Test 15:
  // Purpose:
  //    - Clone matrix_ref
  matrix2<int> m15 = {
      {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
  auto m15_1 = m15(slice{0, 2}, slice{0, 2});
  matrix<int, 2> m15_2 = m15_1.clone();
  m15_1(1, 2) = 69;
  matrix2<int> m15_1_truth = {{1,2,3},{5,6,69},{9,10,11}};
  assert(m15_1 == m15_1_truth);
  matrix2<int> m15_2_truth = {{1,2,3},{5,6,7},{9,10,11}};
  assert(m15_2 == m15_2_truth);
  print_i("15: PASS!\n");
}
