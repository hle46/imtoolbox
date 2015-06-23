#ifndef IMTOOLBOX_PROC
#define IMTOOLBOX_PROC

namespace imtoolbox {
// Average images in a folder
template <typename T>
inline matrix3<T> avg_folder(const char *path_name, const char *avg_file_name,
                             int n) {
  assert(n >= 1);
  std::string path{path_name};
  if (path[path.length() - 1] != '/') {
    path += "/";
  }
  auto ret = imread<T>(path + "1.jpg");
  for (int i = 2; i <= n; ++i) {
    ret += imread<uint8_t>(path + std::to_string(i) + ".jpg");
  }
  ret /= 5;
  imwrite(ret, avg_file_name);
  return ret;
}

// Sum
template <typename M>
inline enable_if_t<is_matrix<M>() && M::order >= 2,
                   matrix<typename M::value_type, M::order - 1>>
sum(const M &mat, size_t dim = M::order - 1) {
  using T = typename M::value_type;
  constexpr size_t N = M::order;

  if (is_empty(mat)) {
    return empty_matrix<T, N - 1>();
  }

  assert(dim <= M::order);
  // Calculate out dimension
  auto in_desc = mat.descriptor();
  matrix_slice<N - 1> out_desc;
  get_matrix_slice(dim, 0, in_desc, out_desc);

  // Contruct result
  matrix<T, N - 1> ret(out_desc, static_cast<T>(0));

  // Loop through dim
  for (size_t i = 0; i < mat.size(dim); ++i) {
    matrix_ref<T, N - 1> mat_ref{out_desc, const_cast<T *>(mat.data())};
    ret += mat_ref;
    out_desc.start += in_desc.strides[dim];
  }

  return ret;
}

template <typename T>
inline matrix<T, 0> sum(const matrix_ref<T, 1> &mr, size_t dim = 0) {
  assert(dim == 0);
  return std::accumulate(mr.begin(), mr.end(), static_cast<T>(0));
}

template <typename T>
inline matrix<T, 0> sum(const matrix<T, 1> &m, size_t dim = 0) {
  assert(dim == 0);
  return std::accumulate(m.begin(), m.end(), static_cast<T>(0));
}

template <typename M>
inline enable_if_t<is_matrix<M>(), typename M::value_type>
sum_all(const M &mat) {
  using value_t = typename M::value_type;
  return std::accumulate(mat.begin(), mat.end(), static_cast<value_t>(0));
}

// filter2
template <typename M, typename H>
enable_if_t<
    is_matrix<M>() && is_matrix<H>() && M::order == 2 && H::order == 2 &&
        is_compatible<typename M::value_type, typename H::value_type>(),
    matrix2<conditional_t<
        is_convertible<typename H::value_type, typename M::value_type>(),
        typename M::value_type, typename H::value_type>>>
filter2_valid(const H &h, const M &m) {
  using value_t = conditional_t<
      is_convertible<typename H::value_type, typename M::value_type>(),
      typename M::value_type, typename H::value_type>;

  if (is_empty(h)) {
    return matrix2<value_t>{m.descriptor(), static_cast<value_t>(0)};
  }

  if (is_empty(m) || (m.rows() <= (h.rows() - 1)) ||
      (m.cols() <= (h.cols() - 1))) {
    return empty_matrix<value_t, 2>();
  }

  matrix2<value_t> ret(m.rows() - (h.rows() - 1), m.cols() - (h.cols() - 1));
  auto ret_ptr = ret.begin();
  for (size_t i = 0; i < ret.rows(); ++i) {
    for (size_t j = 0; j < ret.cols(); ++j) {
      auto m1 = m(slice{i, i + h.rows() - 1}, slice{j, j + h.rows() - 1});
      *ret_ptr = std::inner_product(h.begin(), h.end(), m1.begin(),
                                    static_cast<value_t>(0));
      ++ret_ptr;
    }
  }
  return ret;
}

template <typename M, typename H>
enable_if_t<
    is_matrix<M>() && is_matrix<H>() && M::order == 2 && H::order == 2 &&
        is_compatible<typename M::value_type, typename H::value_type>(),
    matrix2<conditional_t<
        is_convertible<typename H::value_type, typename M::value_type>(),
        typename M::value_type, typename H::value_type>>>
filter2_full(const H &h, const M &m) {
  using value_t = conditional_t<
      is_convertible<typename H::value_type, typename M::value_type>(),
      typename M::value_type, typename H::value_type>;

  if (is_empty(h)) {
    return matrix2<value_t>{m.descriptor(), static_cast<value_t>(0)};
  }

  if (is_empty(m)) {
    return empty_matrix<value_t, 2>();
  }

  matrix2<value_t> m_pad(m.rows() + 2 * (h.rows() - 1),
                         m.cols() + 2 * (h.cols() - 1));
  // Padding
  m_pad(slice{0, h.rows() - 1}, slice::all) = 0;
  m_pad(slice{m.rows() + (h.rows() - 1)}, slice::all) = 0;
  m_pad(slice::all, slice{0, h.cols() - 1}) = 0;
  m_pad(slice::all, slice{m.cols() + (h.cols() - 1)}) = 0;

  m_pad(slice{h.rows() - 1, m.rows() + (h.rows() - 1) - 1},
        slice{h.cols() - 1, m.cols() + (h.cols() - 1) - 1}) = m;

  matrix2<value_t> ret(m.rows() + (h.rows() - 1), m.cols() + (h.cols() - 1));
  auto ret_ptr = ret.begin();
  for (size_t i = 0; i < ret.rows(); ++i) {
    for (size_t j = 0; j < ret.cols(); ++j) {
      auto m1 = m_pad(slice{i, i + h.rows() - 1}, slice{j, j + h.rows() - 1});
      *ret_ptr = std::inner_product(h.begin(), h.end(), m1.begin(),
                                    static_cast<value_t>(0));
      ++ret_ptr;
    }
  }
  return ret;
}

template <typename M, typename H>
enable_if_t<
    is_matrix<M>() && is_matrix<H>() && M::order == 2 && H::order == 2 &&
        is_compatible<typename M::value_type, typename H::value_type>(),
    matrix2<conditional_t<
        is_convertible<typename H::value_type, typename M::value_type>(),
        typename M::value_type, typename H::value_type>>>
filter2_same(const H &h, const M &m) {
  using value_t = conditional_t<
      is_convertible<typename H::value_type, typename M::value_type>(),
      typename M::value_type, typename H::value_type>;

  if (is_empty(h)) {
    return matrix2<value_t>{m.descriptor(), static_cast<value_t>(0)};
  }

  if (is_empty(m)) {
    return empty_matrix<value_t, 2>();
  }

  matrix2<value_t> m_pad(m.rows() + (h.rows() - 1), m.cols() + (h.cols() - 1));
  // Padding
  m_pad(slice{0, (h.rows() - 1) / 2}, slice::all) = 0;
  m_pad(slice{m.rows() + (h.rows() - 1) / 2}, slice::all) = 0;
  m_pad(slice::all, slice{0, (h.cols() - 1) / 2}) = 0;
  m_pad(slice::all, slice{m.cols() + (h.cols() - 1) / 2}) = 0;

  m_pad(slice{(h.rows() - 1) / 2, m.rows() + (h.rows() - 1) / 2 - 1},
        slice{(h.cols() - 1) / 2, m.cols() + (h.cols() - 1) / 2 - 1}) = m;

  matrix2<value_t> ret(m.rows(), m.cols());
  auto ret_ptr = ret.begin();
  for (size_t i = 0; i < ret.rows(); ++i) {
    for (size_t j = 0; j < ret.cols(); ++j) {
      auto m1 = m_pad(slice{i, i + h.rows() - 1}, slice{j, j + h.rows() - 1});
      *ret_ptr = std::inner_product(h.begin(), h.end(), m1.begin(),
                                    static_cast<value_t>(0));
      ++ret_ptr;
    }
  }
  return ret;
}

enum class filter2_t { valid, same, full };

template <typename M, typename H>
enable_if_t<
    is_matrix<M>() && is_matrix<H>() && M::order == 2 && H::order == 2 &&
        is_compatible<typename M::value_type, typename H::value_type>(),
    matrix2<conditional_t<
        is_convertible<typename H::value_type, typename M::value_type>(),
        typename M::value_type, typename H::value_type>>>
filter2(const H &h, const M &m, filter2_t ft = filter2_t::same) {
  switch (ft) {
  case filter2_t::valid:
    return filter2_valid(h, m);
  case filter2_t::full:
    return filter2_full(h, m);
  case filter2_t::same:
    return filter2_same(h, m);
  }
}

// findpeaks
template <typename InputIt>
enable_if_t<is_integral<typename std::iterator_traits<InputIt>::value_type>(),
            std::vector<std::pair<
                typename std::iterator_traits<InputIt>::value_type, size_t>>>
findpeaks(InputIt first, InputIt last) {
  std::vector<std::pair<typename std::iterator_traits<InputIt>::value_type,
                        size_t>> pks;
  InputIt back = first;
  InputIt curr = ++first;
  InputIt next = ++first;
  auto inc_iters = [&]() {
    ++back;
    ++curr;
    ++next;
  };
  size_t n = 1;
  while (next != last) {
    if (*curr < *back) {
      inc_iters();
      ++n;
      continue;
    }
    size_t i = 1;
    while (*curr == *next) {
      inc_iters();
      ++i;
    }
    if (*curr > *next) {
      pks.emplace_back(*curr, n);
    }
    inc_iters();
    n += i;
  }
  return pks;
}

template <typename InputIt>
enable_if_t<
    is_floating_point<typename std::iterator_traits<InputIt>::value_type>(),
    std::vector<
        std::pair<typename std::iterator_traits<InputIt>::value_type, size_t>>>
findpeaks(InputIt first, InputIt last) {
  std::vector<std::pair<typename std::iterator_traits<InputIt>::value_type,
                        size_t>> pks;
  InputIt back = first;
  InputIt curr = ++first;
  InputIt next = ++first;
  auto inc_iters = [&]() {
    ++back;
    ++curr;
    ++next;
  };
  size_t n = 1;
  while (next != last) {
    if (*curr < *back || almost_equal(*curr, *back)) {
      inc_iters();
      ++n;
      continue;
    }
    size_t i = 1;
    while (almost_equal(*curr, *next)) {
      inc_iters();
      ++i;
    }
    if (*curr > *next) {
      pks.emplace_back(*curr, n);
    }
    inc_iters();
    n += i;
  }
  return pks;
}

template <typename V>
enable_if_t<is_matrix<V>() && V::order == 1 &&
                is_arithmetic<typename V::value_type>(),
            std::pair<typename V::value_type, size_t>>
findpeaks(const V &v, sort_t st,
          typename V::value_type thr =
              std::numeric_limits<typename V::value_type>::lowest()) {
  auto pks = findpeaks(v.begin(), v.end());
  using T = typename V::value_type;
  using P = typename std::pair<T, size_t>;
  switch (st) {
  case sort_t::descend:
    std::sort(pks.begin(), pks.end(),
              [](const P &a, const P &b) { return a.first > b.first; });
    break;
  case sort_t::ascend:
    std::sort(pks.begin(), pks.end(),
              [](const P &a, const P &b) { return a.first < b.first; });
    break;
  case sort_t::none:
    // Do nothing
    break;
  }

  // Erase remove idiom
  pks.erase(std::remove_if(pks.begin(), pks.end(), [&](const P &p) {
    p.first < thr;
  }), pks.end());
  return pks;
}

template <typename T> struct g_slice {
  T start;
  T end;
  T inc;
  explicit g_slice(T s, T e, T n = static_cast<T>(1))
      : start(s), end(e), inc(n) {
    assert(s <= e);
  }
};

template <typename T>
enable_if_t<is_arithmetic<T>(), std::pair<matrix2<T>, matrix2<T>>>
meshgrid(const g_slice<T> &gs1, const g_slice<T> &gs2) {
  size_t w = (gs1.end - gs1.start) / gs1.inc + 1;
  size_t h = (gs2.end - gs2.start) / gs2.inc + 1;
  matrix2<T> x(h, w);
  auto iter = x.begin();
  T val = gs1.start;
  T inc = gs1.inc;
  for (size_t i = 0; i < h; ++i) {
    val = gs1.start;
    for (size_t j = 0; j < w; ++j) {
      *iter = val;
      val += inc;
      ++iter;
    }
  }

  matrix2<T> y(h, w);
  iter = y.begin();
  val = gs2.start;
  inc = gs2.inc;
  for (size_t i = 0; i < h; ++i) {
    for (size_t j = 0; j < w; ++j) {
      *iter = val;
      ++iter;
    }
    val += inc;
  }

  return std::make_pair(std::move(x), std::move(y));
}

template <typename T, size_t N, typename... Args>
inline matrix<T, N> zeros(Args... args) {
  matrix<T, N> m(args...);
  std::fill(m.begin(), m.end(), static_cast<T>(0));
  return m;
}

template <typename T, size_t N, typename... Args>
inline matrix<T, N> ones(Args... args) {
  matrix<T, N> m(args...);
  std::fill(m.begin(), m.end(), static_cast<T>(1));
  return m;
}

template <typename T>
inline enable_if_t<is_floating_point<T>(), matrix2<T>>
fspecial_gaussian(const std::pair<int, int> &siz, T std) {
  assert(siz.first >= 0 && siz.second >= 0);
  T siz1 = (siz.first - 1) / 2.0;
  T siz2 = (siz.second - 1) / 2.0;

  matrix2<T> h(siz.first, siz.second);
  auto h_iter = h.begin();
  T sigma = 2 * std * std;
  for (T x = -siz1; x <= siz1; ++x) {
    for (T y = -siz2; y <= siz2; ++y) {
      *h_iter = std::exp(-(x * x + y * y) / sigma);
      ++h_iter;
    }
  }

  T sumh = sum_all(h);
  h /= sumh;
  return h;
}

template <typename T>
inline enable_if_t<is_floating_point<T>(), matrix2<T>>
fspecial_average(const std::pair<int, int> &siz) {
  matrix2<T> h = ones<T, 2>(siz.first, siz.second);
  T sumh = h.size(0) * h.size(1);
  h /= sumh;
  return h;
}

} // namespace imtoolbox
#endif
