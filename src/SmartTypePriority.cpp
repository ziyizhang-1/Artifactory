#include <array>
#include <iostream>
#include <iterator>
#include <random>
#include <type_traits>

#include "argparse.h"

using namespace std;

static int ratio[4] = {1, 3, 4, 1};

template <typename T, typename S, typename RT = decay_t<common_type_t<T, S>>>
inline RT max(T a, S b) {
  return a > b ? a : b;
}

template <typename T, int N, typename S = float>
class SmartTypePriority {
 private:
  array<S, N> ratio;
  decltype(ratio) counter;
  decltype(ratio) target;

 public:
  explicit SmartTypePriority(T (*ratio)[N])
      : ratio(to_array(*reinterpret_cast<S (*)[N]>(
            const_cast<remove_const_t<T> (*)[N]>(ratio)))) {
    counter.fill(0.0);
    target.fill(0.0);
  }

  SmartTypePriority(const SmartTypePriority&) = delete;

  SmartTypePriority(SmartTypePriority&& other) {
    ratio = move(other.ratio);
    counter = move(other.counter);
    target = move(other.target);
    other.reset();
  }

  ~SmartTypePriority() noexcept = default;

  void update(int type_id, int num) {
    counter[type_id] += num;
    for (int i = 0; i < target.size(); ++i) {
      target[i] =
          ::max(target[i], counter[type_id] * ratio[i] / ratio[type_id]);
    }
  }

  void reset() {
    memset(&counter, (S)0, sizeof(S) * N);
    memset(&target, (S)0, sizeof(S) * N);
  }

  auto info() const noexcept {
    auto res = target;
    for (int i = 0; i < res.size(); ++i) {
      res[i] -= counter[i];
    }
    return res;
  }
};

int main(int argc, char* argv[]) {
  argparse::ArgumentParser program("STP");
  argparse::ArgumentParser simulate_command("simulate");
  simulate_command.add_argument("quantity")
      .help("quantity to be simulated")
      .scan<'i', int>();
  program.add_argument("-r", "--ratio")
      .help("set custom type ratio")
      .default_value("1:3:4:1");
  program.add_subparser(simulate_command);
  auto ratio = const_cast<int (*)[4]>(&::ratio);

  try {
    program.parse_args(argc, argv);
  } catch (const exception& err) {
    cerr << err.what() << endl;
    cerr << program;
    return 1;
  }
  if (program.is_used("--ratio")) {
    auto tmp = program.get<string>("--ratio");
    for (int i = 0, j = 0, k = 0; i < sizeof(::ratio) / sizeof(::ratio[0]);
         ++i) {
      while (j <= tmp.length()) {
        if (tmp[j] == ':' || j == tmp.length()) {
          (*ratio)[i] = stoi(tmp.substr(k, j - k));
          ++j;
          k = j;
          break;
        }
        ++j;
      }
    }
    cout << "Custom Ratio: ";
    copy(*ratio, *(ratio + 1), ostream_iterator<int>(cout, ":"));
    cout << endl;
  }
  if (program.is_subcommand_used(simulate_command)) {
    if (simulate_command.is_used("quantity")) {
      cout << "Simulating quantity: " << simulate_command.get<int>("quantity")
           << endl;
    }
  }

  random_device rd;
  default_random_engine gen(rd());
  uniform_int_distribution<int> u_dist(0, 9);

  cout << "random generated number: " << u_dist(gen) << endl;

  auto stp = SmartTypePriority(ratio);
  unsigned int ty, qty;

  while (true) {
    cout << "Wafer Type: ";
    cin >> ty;
    if (cin.fail() || ty > 4 || ty < 1) {
      cout << "invalid wafer type" << endl;
      cin.clear();
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      continue;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Qty: ";
    cin >> qty;
    if (cin.fail() || qty > 25) {
      cout << "invalid wafer qty" << endl;
      cin.clear();
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      continue;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    stp.update(ty - 1, qty);
    auto res = stp.info();
    move(res.begin(), res.end(), ostream_iterator<float>(cout, " "));
    cout << endl;
  }
}