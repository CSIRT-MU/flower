#include <array>

namespace Async {

template<typename T, std::size_t N = 512>
class Queue {

  class Node {

    std::array<T, N> _values;
    std::size_t _reader = 0;
    volatile std::size_t _writer = 0;

  public:

    Node* volatile _next = nullptr;

    bool full() const {
      return _writer == N;
    }

    bool empty() const {
      return _reader == _writer;
    }

    bool read_end() const {
      return _reader == N;
    }

    void add(T&& value) {
      _values[_writer] = std::move(value);
      ++_writer;
    }

    T pop() {
      while (empty());
      ++_reader;
      return std::move(_values[_reader - 1]);
    }
  };

  Node* volatile _reader = new Node();
  Node* volatile _writer = _reader;

public:

  template<typename TT>
  void push(TT&& value) {
    Node* writer = _writer;

    if (writer->full()) {
      Node* node = new Node();
      writer->_next = node;
      _writer = node;
    }

    _writer->add(std::forward<TT>(value));
  }

  T pop() {
    Node* node = _reader;

    if (node->read_end()) {
      _reader = node->_next;
      delete node;
    }

    return _reader->pop();
  }

  bool empty() const {
    return _reader == _writer && _reader->empty();
  }

};

} // namespace Async
