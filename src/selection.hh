#ifndef selection_hh_INCLUDED
#define selection_hh_INCLUDED

#include "buffer.hh"

namespace Kakoune
{

using CaptureList = Vector<String>;

// A selection is a Selection, associated with a CaptureList
struct Selection
{
    static constexpr MemoryDomain Domain = MemoryDomain::Selections;

    Selection() = default;
    Selection(ByteCoord pos) : Selection(pos,pos) {}
    Selection(ByteCoord anchor, ByteCoord cursor,
              CaptureList captures = {})
        : m_anchor{anchor}, m_cursor{cursor},
          m_captures(std::move(captures)) {}

    void merge_with(const Selection& range);

    ByteCoord& anchor() { return m_anchor; }
    ByteCoordAndTarget& cursor() { return m_cursor; }

    const ByteCoord& anchor() const { return m_anchor; }
    const ByteCoordAndTarget& cursor() const { return m_cursor; }

    CaptureList& captures() { return m_captures; }
    const CaptureList& captures() const { return m_captures; }

    bool operator== (const Selection& other) const
    {
        return m_anchor == other.m_anchor and m_cursor == other.m_cursor;
    }

    const ByteCoord& min() const { return m_anchor < m_cursor ? m_anchor : m_cursor; }
    const ByteCoord& max() const { return m_anchor < m_cursor ? m_cursor : m_anchor; }

    ByteCoord& min() { return m_anchor < m_cursor ? m_anchor : m_cursor; }
    ByteCoord& max() { return m_anchor < m_cursor ? m_cursor : m_anchor; }

private:
    ByteCoord m_anchor;
    ByteCoordAndTarget m_cursor;

    CaptureList m_captures;
};

inline bool overlaps(const Selection& lhs, const Selection& rhs)
{
    return lhs.min() <= rhs.min() ? lhs.max() >= rhs.min()
                                  : lhs.min() <= rhs.max();
}

enum class InsertMode : unsigned
{
    Insert,
    InsertCursor,
    Append,
    Replace,
    InsertAtLineBegin,
    InsertAtNextLineBegin,
    AppendAtLineEnd,
    OpenLineBelow,
    OpenLineAbove
};

struct SelectionList
{
    static constexpr MemoryDomain Domain = MemoryDomain::Selections;

    SelectionList(Buffer& buffer, Selection s);
    SelectionList(Buffer& buffer, Selection s, size_t timestamp);
    SelectionList(Buffer& buffer, Vector<Selection> s);
    SelectionList(Buffer& buffer, Vector<Selection> s, size_t timestamp);

    void update();

    void check_invariant() const;

    const Selection& main() const { return (*this)[m_main]; }
    Selection& main() { return (*this)[m_main]; }
    size_t main_index() const { return m_main; }
    void set_main_index(size_t main) { kak_assert(main < size()); m_main = main; }

    void rotate_main(int count) { m_main = (m_main + count) % size(); }

    void avoid_eol();

    void push_back(const Selection& sel) { m_selections.push_back(sel); }
    void push_back(Selection&& sel) { m_selections.push_back(std::move(sel)); }

    Selection& operator[](size_t i) { return m_selections[i]; }
    const Selection& operator[](size_t i) const { return m_selections[i]; }

    SelectionList& operator=(Vector<Selection> list)
    {
        m_selections = std::move(list);
        m_main = size()-1;
        sort_and_merge_overlapping();
        update_timestamp();
        check_invariant();
        return *this;
    }

    using iterator = Vector<Selection>::iterator;
    iterator begin() { return m_selections.begin(); }
    iterator end() { return m_selections.end(); }

    using const_iterator = Vector<Selection>::const_iterator;
    const_iterator begin() const { return m_selections.begin(); }
    const_iterator end() const { return m_selections.end(); }

    void remove(size_t index) { m_selections.erase(begin() + index); }

    size_t size() const { return m_selections.size(); }

    bool operator==(const SelectionList& other) const { return m_buffer == other.m_buffer and m_selections == other.m_selections; }
    bool operator!=(const SelectionList& other) const { return !((*this) == other); }

    void sort_and_merge_overlapping();

    Buffer& buffer() const { return *m_buffer; }

    size_t timestamp() const { return m_timestamp; }
    void update_timestamp() { m_timestamp = m_buffer->timestamp(); }

    void insert(ConstArrayView<String> strings, InsertMode mode,
                bool select_inserted = false);
    void erase();

private:
    size_t m_main = 0;
    Vector<Selection> m_selections;

    SafePtr<Buffer> m_buffer;
    size_t m_timestamp;
};

Vector<Selection> compute_modified_ranges(Buffer& buffer, size_t timestamp);

String selection_to_string(const Selection& selection);
String selection_list_to_string(const SelectionList& selection);
Selection selection_from_string(StringView desc);
SelectionList selection_list_from_string(Buffer& buffer, StringView desc);

}

#endif // selection_hh_INCLUDED
