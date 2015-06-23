#ifndef IMTOOLBOX_IMAGE_H
#define IMTOOLBOX_IMAGE_H
#include <cmath>
#include <cstring>
#include <cstdio>
#include <jpeglib.h>
#include <setjmp.h>
#include <png.h>

namespace imtoolbox {

struct my_error_mgr {
  struct jpeg_error_mgr pub; /* "public" fields */
  jmp_buf setjmp_buffer;     /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

// template version of imread_jpeg
template <typename T> matrix3<T> imread_jpeg(const char *file_name) {
  FILE *fp{fopen(file_name, "rb")}; // Open file
  if (fp == nullptr) {
    print_e("File not found\n");
    throw std::exception();
  }

  struct jpeg_decompress_struct cinfo; // JPEG decompression parameters
  struct my_error_mgr jerr;            // Private extension JPEG error handler
  cinfo.err = jpeg_std_error(&jerr.pub);

  // Install custom error handler
  jerr.pub.error_exit = [](j_common_ptr cinfo) {
    char jmsg[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, jmsg);
    print_e(jmsg, "\n");
    my_error_ptr myerr =
        (my_error_ptr)
            cinfo->err; // Hack! err points to the beginning of my_error_ptr
    longjmp(myerr->setjmp_buffer, 1);
  };

  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
    throw std::exception();
  }

  jpeg_create_decompress(&cinfo); // Initialize the JPEG decompression object

  jpeg_stdio_src(&cinfo, fp); // Specify data source

  jpeg_read_header(&cinfo, TRUE); // Read file parameters

  jpeg_start_decompress(&cinfo); // Start decompressor

  matrix3<T> mat(cinfo.output_height, cinfo.output_width,
                 cinfo.output_components); // Return result

  auto row_stride = cinfo.output_width * cinfo.output_components;

  // jpeg_read_scanlines expects an array of pointers to scanlines.
  // Here we create an array of one element long
  JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo,
                                                 JPOOL_IMAGE, row_stride, 1);

  auto pix_ptr = mat.data();
  // Scan line by line
  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, buffer, 1);
    uint8_t *buf_ptr{buffer[0]};
    for (uint32_t i = 0; i < row_stride; ++i) {
      *pix_ptr = *buf_ptr;
      ++pix_ptr;
      ++buf_ptr;
    }
  }

  jpeg_finish_decompress(&cinfo); // Finish decompression

  jpeg_destroy_decompress(&cinfo);
  fclose(fp);
  return mat;
}

// uint8_t specialization of imread_jpeg - no need to allocate buffer array
template <> matrix3<uint8_t> imread_jpeg(const char *file_name) {
  FILE *fp{fopen(file_name, "rb")}; // Open file
  if (fp == nullptr) {
    print_e("File not found\n");
    throw std::exception();
  }

  struct jpeg_decompress_struct cinfo; // JPEG decompression parameters
  struct my_error_mgr jerr;            // Private extension JPEG error handler
  cinfo.err = jpeg_std_error(&jerr.pub);

  // Install custom error handler
  jerr.pub.error_exit = [](j_common_ptr cinfo) {
    char jmsg[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, jmsg);
    print_e(jmsg, "\n");
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    longjmp(myerr->setjmp_buffer, 1);
  };
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
    throw std::exception();
  }

  jpeg_create_decompress(&cinfo); // Initialize the JPEG decompression object

  jpeg_stdio_src(&cinfo, fp); // Specify data source

  jpeg_read_header(&cinfo, TRUE); // Read file parameters

  jpeg_start_decompress(&cinfo); // Start decompressor

  println_i(typeid(cinfo.output_components).name());
  matrix3<uint8_t> mat(cinfo.output_height, cinfo.output_width,
                       cinfo.output_components); // Return result

  auto row_stride = cinfo.output_width * cinfo.output_components;

  auto pix_ptr = mat.data();
  // Scan line by line
  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, &pix_ptr, 1);
    pix_ptr += row_stride;
  }

  jpeg_finish_decompress(&cinfo); // Finish decompression

  jpeg_destroy_decompress(&cinfo);
  fclose(fp);
  return mat;
}

template <typename M>
enable_if_t<is_matrix<M>() && !is_same<typename M::value_type, uint8_t>(), void>
imwrite_jpeg(const M &mat, const char *file_name, uint quality = 100) {
  static_assert(M::order == 2 || M::order == 3, "Invalid image");
  FILE *fp{fopen(file_name, "wb")}; // Open file
  if (fp == nullptr) {
    print_e("File not found\n");
    throw std::exception();
  }

  struct jpeg_compress_struct cinfo; // JPEG decompression parameters
  struct my_error_mgr jerr;          // Private extension JPEG error handler
  cinfo.err = jpeg_std_error(&jerr.pub);

  // Install custom error handler
  jerr.pub.error_exit = [](j_common_ptr cinfo) {
    char jmsg[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, jmsg);
    print_e(jmsg, "\n");
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    longjmp(myerr->setjmp_buffer, 1);
  };
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_compress(&cinfo);
    fclose(fp);
    throw std::exception();
  }

  jpeg_create_compress(&cinfo); // Initialize JPEG compression object

  jpeg_stdio_dest(&cinfo, fp); // Specify data destination

  cinfo.image_height = mat.size(0);                  // Specify image height
  cinfo.image_width = mat.size(1);                   // Specify image width
  cinfo.input_components = (mat.order == 2) ? 1 : 3; // Specify image components
  cinfo.in_color_space =
      (mat.order == 2) ? JCS_GRAYSCALE : JCS_RGB; // Specify color space
  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, true);

  jpeg_start_compress(&cinfo, true);

  auto row_stride = cinfo.image_width * cinfo.input_components;
  std::vector<uint8_t> buffer(row_stride);
  auto buf_ptr = buffer.data();
  auto pix_ptr = mat.data();
  while (cinfo.next_scanline < cinfo.image_height) {
    // Round and copy data to buffer
    for (size_t i = 0; i < row_stride; ++i) {
      buffer[i] = std::round(*pix_ptr++);
    }
    jpeg_write_scanlines(&cinfo, &buf_ptr, 1);
  }

  jpeg_finish_compress(&cinfo);

  jpeg_destroy_compress(&cinfo);

  fclose(fp);
}

template <typename M>
enable_if_t<is_matrix<M>() && is_same<typename M::value_type, uint8_t>(), void>
imwrite_jpeg(const M &mat, const char *file_name, uint quality = 100) {
  static_assert(M::order == 2 || M::order == 3, "Invalid image");
  FILE *fp{fopen(file_name, "wb")}; // Open file
  if (fp == nullptr) {
    print_e("File not found\n");
    throw std::exception();
  }

  struct jpeg_compress_struct cinfo; // JPEG decompression parameters
  struct my_error_mgr jerr;          // Private extension JPEG error handler
  cinfo.err = jpeg_std_error(&jerr.pub);

  // Install custom error handler
  jerr.pub.error_exit = [](j_common_ptr cinfo) {
    char jmsg[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, jmsg);
    print_e(jmsg, "\n");
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    longjmp(myerr->setjmp_buffer, 1);
  };
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_compress(&cinfo);
    fclose(fp);
    throw std::exception();
  }

  jpeg_create_compress(&cinfo); // Initialize JPEG compression object

  jpeg_stdio_dest(&cinfo, fp); // Specify data destination

  cinfo.image_height = mat.size(0);                  // Specify image height
  cinfo.image_width = mat.size(1);                   // Specify image width
  cinfo.input_components = (mat.order == 2) ? 1 : 3; // Specify image components
  cinfo.in_color_space =
      (mat.order == 2) ? JCS_GRAYSCALE : JCS_RGB; // Specify color space
  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, true);

  jpeg_start_compress(&cinfo, true);

  auto row_stride = cinfo.image_width * cinfo.input_components;
  uint8_t *pix_ptr{const_cast<uint8_t *>(mat.data())};
  while (cinfo.next_scanline < cinfo.image_height) {
    jpeg_write_scanlines(&cinfo, &pix_ptr, 1);
    pix_ptr += row_stride;
  }

  jpeg_finish_compress(&cinfo);

  jpeg_destroy_compress(&cinfo);

  fclose(fp);
}

template <typename T> matrix3<T> imread_png(const char *file_name) {
  FILE *fp{fopen(file_name, "rb")};
  if (fp == nullptr) {
    print_e("File not found\n");
    throw std::exception();
  }

  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr,
                             nullptr); // Create and initialize the png_struct
  if (png_ptr == nullptr) {
    fclose(fp);
    throw std::exception();
  }

  png_infop info_ptr = png_create_info_struct(
      png_ptr); // Allocate and initialize the memory for image information
  if (info_ptr == nullptr) {
    /* Free all of the memory associated with the png_ptr and info_ptr */
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    fclose(fp);
    throw std::exception();
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    /* Free all of the memory associated with the png_ptr and info_ptr */
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(fp);
    /* If we get here, we had a problem reading the file */
    throw std::exception();
  }

  /* Set up the input control if you are using standard C streams */
  png_init_io(png_ptr, fp);

  png_read_info(png_ptr, info_ptr); // Get all of the information from PNG file
                                    // before the first image data chunk (IDAT)

  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, nullptr, nullptr);

  if (bit_depth != 8) {
    print_e("Not support bit depth different than 8 for now\n");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  if (interlace_type != PNG_INTERLACE_NONE) {
    print_e("Not support interlace for now\n");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  uint32_t depth;
  switch (color_type) {
  case PNG_COLOR_TYPE_GRAY:
    depth = 1;
    break;
  case PNG_COLOR_TYPE_RGB:
    depth = 3;
    break;
  default:
    print_e("Color type is not supported\n");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  uint32_t row_stride = width * depth;

  std::vector<uint8_t> buffer(row_stride);

  matrix3<T> mat(height, width, depth);
  auto pix_ptr = mat.data();

  for (uint32_t i = 0; i < height; ++i) {
    auto buf_ptr = buffer.data();
    png_read_rows(png_ptr, &buf_ptr, nullptr, 1);
    // Copy to mat
    for (uint32_t j = 0; j < row_stride; ++j) {
      *pix_ptr = *buf_ptr;
      ++pix_ptr;
      ++buf_ptr;
    }
  }

  // Read rest of file, and get additional chunks in info_ptr
  png_read_end(png_ptr, info_ptr);

  // Clean up after the read, and free any memory allocated
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  // Close the file
  fclose(fp);
  return mat;
}

template <> matrix3<uint8_t> imread_png(const char *file_name) {
  FILE *fp{fopen(file_name, "rb")};
  if (fp == nullptr) {
    print_e("File not found\n");
    throw std::exception();
  }

  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr,
                             nullptr); // Create and initialize the png_struct
  if (png_ptr == nullptr) {
    fclose(fp);
    throw std::exception();
  }

  png_infop info_ptr = png_create_info_struct(
      png_ptr); // Allocate and initialize the memory for image information
  if (info_ptr == nullptr) {
    /* Free all of the memory associated with the png_ptr and info_ptr */
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    fclose(fp);
    throw std::exception();
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    /* Free all of the memory associated with the png_ptr and info_ptr */
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(fp);
    /* If we get here, we had a problem reading the file */
    throw std::exception();
  }

  /* Set up the input control if you are using standard C streams */
  png_init_io(png_ptr, fp);

  png_read_info(png_ptr, info_ptr); // Get all of the information from PNG file
                                    // before the first image data chunk (IDAT)

  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, nullptr, nullptr);

  if (bit_depth != 8) {
    print_e("Not support bit depth different than 8 for now\n");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  if (interlace_type != PNG_INTERLACE_NONE) {
    print_e("Not support interlace for now\n");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  uint32_t depth;
  switch (color_type) {
  case PNG_COLOR_TYPE_GRAY:
    depth = 1;
    break;
  case PNG_COLOR_TYPE_RGB:
    depth = 3;
    break;
  default:
    print_e("Color type is not supported\n");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  uint32_t row_stride = width * depth;

  matrix3<uint8_t> mat(height, width, depth);
  auto pix_ptr = mat.data();

  for (uint32_t i = 0; i < height; ++i) {
    png_read_rows(png_ptr, &pix_ptr, nullptr, 1);
    pix_ptr += row_stride;
  }

  // Read rest of file, and get additional chunks in info_ptr
  png_read_end(png_ptr, info_ptr);

  // Clean up after the read, and free any memory allocated
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  // Close the file
  fclose(fp);
  return mat;
}

template <typename M>
enable_if_t<is_matrix<M>() && !is_same<typename M::value_type, uint8_t>(), void>
imwrite_png(const M &mat, const char *file_name) {
  static_assert(M::order == 2 || M::order == 3, "Invalid image");
  FILE *fp{fopen(file_name, "wb")};
  if (fp == nullptr) {
    print_e("File not found\n");
    throw std::exception();
  }

  // Create and initialize the png_struct with the desired error handler
  // functions.
  png_structp png_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (png_ptr == nullptr) {
    fclose(fp);
    throw std::exception();
  }

  // Allocate/initialize the image information data.
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr) {
    png_destroy_write_struct(&png_ptr, nullptr);
    fclose(fp);
    throw std::exception();
  }

  // Set error handling
  if (setjmp(png_jmpbuf(png_ptr))) {
    // If we get here, we had a problem writing the file
    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    throw std::exception();
  }

  // Set up the output control if you are using standard C streams
  png_init_io(png_ptr, fp);

  png_uint_32 height = mat.size(0), width = mat.size(1);

  if (height > PNG_UINT_32_MAX / (sizeof(png_bytep))) {
    png_error(png_ptr, "Image is too tall to process in memory");
  }

  int color_type;
  size_t depth = mat.size(2);
  switch (depth) {
  case 1:
    color_type = PNG_COLOR_TYPE_GRAY;
    break;
  case 3:
    color_type = PNG_COLOR_TYPE_RGB;
    break;
  default:
    print_e("Invalid color type\n");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  png_set_IHDR(png_ptr, info_ptr, width, height, 8, color_type,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
               PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);

  auto row_stride = width * depth;
  std::vector<uint8_t> buffer(row_stride);
  uint8_t *buf_ptr = buffer.data();
  auto pix_ptr = mat.data();
  for (uint32_t i = 0; i < height; ++i) {
    // Round and copy data to buffer
    for (size_t j = 0; j < row_stride; ++j) {
      buffer[j] = std::round(*pix_ptr++);
    }
    png_write_rows(png_ptr, &buf_ptr, 1);
  }

  png_write_end(png_ptr, info_ptr);

  // Clean up after the write, and free any memory allocated
  png_destroy_write_struct(&png_ptr, &info_ptr);

  // Close the file
  fclose(fp);
}

template <typename M>
enable_if_t<is_matrix<M>() && is_same<typename M::value_type, uint8_t>(), void>
imwrite_png(const M &mat, const char *file_name) {
  static_assert(M::order == 2 || M::order == 3, "Invalid image");
  FILE *fp{fopen(file_name, "wb")};
  if (fp == nullptr) {
    print_e("File not found\n");
    throw std::exception();
  }

  // Create and initialize the png_struct with the desired error handler
  // functions.
  png_structp png_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (png_ptr == nullptr) {
    fclose(fp);
    throw std::exception();
  }

  // Allocate/initialize the image information data.
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr) {
    png_destroy_write_struct(&png_ptr, nullptr);
    fclose(fp);
    throw std::exception();
  }

  // Set error handling
  if (setjmp(png_jmpbuf(png_ptr))) {
    // If we get here, we had a problem writing the file
    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    throw std::exception();
  }

  // Set up the output control if you are using standard C streams
  png_init_io(png_ptr, fp);

  png_uint_32 height = mat.size(0), width = mat.size(1);

  if (height > PNG_UINT_32_MAX / (sizeof(png_bytep))) {
    png_error(png_ptr, "Image is too tall to process in memory");
  }

  int color_type;
  size_t depth = mat.size(2);
  switch (depth) {
  case 1:
    color_type = PNG_COLOR_TYPE_GRAY;
    break;
  case 3:
    color_type = PNG_COLOR_TYPE_RGB;
    break;
  default:
    print_e("Invalid color type\n");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  png_set_IHDR(png_ptr, info_ptr, width, height, 8, color_type,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
               PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);

  auto row_stride = width * depth;
  uint8_t *pix_ptr{const_cast<uint8_t *>(mat.data())};
  for (uint32_t i = 0; i < height; ++i) {
    png_write_rows(png_ptr, &pix_ptr, 1);
    pix_ptr += row_stride;
  }

  png_write_end(png_ptr, info_ptr);

  // Clean up after the write, and free any memory allocated
  png_destroy_write_struct(&png_ptr, &info_ptr);

  // Close the file
  fclose(fp);
}

bool inline is_sig_equal(const uint8_t *sig1, const uint8_t *sig2,
                         size_t num_to_check) {
  return (memcmp(sig1, sig2, num_to_check) == 0);
}

bool is_png(const uint8_t *sig) {
  constexpr int png_bytes_to_check = 8;
  constexpr uint8_t png_sig[png_bytes_to_check] = {137, 80, 78, 71,
                                                   13,  10, 26, 10};
  return is_sig_equal(sig, png_sig, png_bytes_to_check);
}

bool is_jpeg(const uint8_t *sig) {
  constexpr int jpeg_bytes_to_check = 4;
  constexpr int num_jpeg_sigs = 3;
  constexpr uint8_t jpeg_sig[num_jpeg_sigs][jpeg_bytes_to_check] = {
      {255, 216, 255, 224}, {255, 216, 255, 225}, {255, 216, 255, 232}};

  for (int i = 0; i < num_jpeg_sigs; ++i) {
    if (is_sig_equal(sig, jpeg_sig[i], jpeg_bytes_to_check)) {
      return true;
    }
  }
  return false;
}

template <typename T> inline matrix3<T> imread(const char *file_name) {
  FILE *fp{fopen(file_name, "rb")};
  if (fp == nullptr) {
    print_e("File not found\n");
    throw std::exception();
  }

  constexpr int max_bytes = 8;
  uint8_t sig[max_bytes];
  if (fread(sig, 1, max_bytes, fp) != max_bytes) {
    fclose(fp);
    throw std::exception();
  }

  fclose(fp);

  if (is_png(sig)) {
    return imread_png<T>(file_name);
  } else if (is_jpeg(sig)) {
    return imread_jpeg<T>(file_name);
  } else {
    print_e("File type isn't supported\n");
    throw std::exception();
  }
}

template <typename T>
inline matrix3<T> imread(const std::string &file_name) {
  return imread<T>(file_name.c_str());
}

template <typename M>
enable_if_t<is_matrix<M>(), void> imwrite(const M &mat,
                                          const char *file_name) {
  static_assert(M::order == 2 || M::order == 3, "Invalid image");
  const char *pch = strchr(file_name, '.');
  const char *last_pch = nullptr;
  while (pch != nullptr) {
    last_pch = pch;
    pch = strchr(pch + 1, '.');
  }

  if (last_pch == nullptr) {
    print_e("No extension supplied\n");
    throw std::exception();
  }

  last_pch += 1;

  if (strncmp(last_pch, "png", 3) == 0) {
    imwrite_png(mat, file_name);
  } else if (strncmp(last_pch, "jpg", 3) == 0 ||
             strncmp(last_pch, "jpeg", 4) == 0) {
    imwrite_jpeg(mat, file_name);
  } else {
    print_e("Unknown extension\n");
    throw std::exception();
  }
}
} // namespace imtoolbox
#endif // IMTOOLBOX_IMAGE_H
