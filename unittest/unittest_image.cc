#include <iostream>
#include "../imtoolbox.h"
using namespace imtoolbox;
int main() {
  // Test 1:
  // General read jpeg function
  auto mat = imread<int>("1.jpg");
  assert(mat(428, 922, 0) == 3 && mat(428, 922, 1) == 98 &&
         mat(428, 922, 2) == 16);
  assert(mat(1130, 532, 0) == 2 && mat(1130, 532, 1) == 68 &&
         mat(1130, 532, 2) == 7);
  assert(mat(580, 816, 0) == 2 && mat(580, 816, 1) == 44 &&
         mat(580, 816, 2) == 4);
  assert(mat(640, 2012, 0) == 1 && mat(640, 2012, 1) == 110 &&
         mat(640, 2012, 2) == 19);

  // Test 2:
  // uint8_t specialization version
  auto mat1 = imread<uint8_t>("1.jpg");
  assert(mat1(428, 922, 0) == 3 && mat1(428, 922, 1) == 98 &&
         mat1(428, 922, 2) == 16);
  assert(mat1(1130, 532, 0) == 2 && mat1(1130, 532, 1) == 68 &&
         mat1(1130, 532, 2) == 7);
  assert(mat1(580, 816, 0) == 2 && mat1(580, 816, 1) == 44 &&
         mat1(580, 816, 2) == 4);
  assert(mat1(640, 2012, 0) == 1 && mat1(640, 2012, 1) == 110 &&
         mat1(640, 2012, 2) == 19);

  // Test 3:
  // Ability to write jpeg
  imwrite(mat, "test.jpg");
  imwrite(mat1, "test1.jpg");

  // Test 4:
  auto mat2 = imread<int>("avg.png");
  assert(mat2(790, 1288, 0) == 2 && mat2(790, 1288, 1) == 157 &&
         mat2(790, 1288, 2) == 36);

  // Test 5:
  auto mat3 = imread<uint8_t>("avg.png");
  assert(mat3(790, 1288, 0) == 2 && mat3(790, 1288, 1) == 157 &&
         mat3(790, 1288, 2) == 36);
 
  // Test 6:
  imwrite(mat2, "test_avg.png");
  auto mat4 = imread<int>("test_avg.png");
  assert(mat4(790, 1288, 0) == 2 && mat4(790, 1288, 1) == 157 &&
         mat4(790, 1288, 2) == 36);

  // Test 7
  imwrite(mat3, "test_avg1.png");
  auto mat5 = imread<uint8_t>("test_avg1.png");
  assert(mat5(790, 1288, 0) == 2 && mat5(790, 1288, 1) == 157 &&
         mat5(790, 1288, 2) == 36);
    
  return 0;
}
