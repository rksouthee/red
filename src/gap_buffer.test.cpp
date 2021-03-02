#include "gap_buffer.h"
#include <cassert>

int main()
{
	Gap_buffer x;
	x.insert(x.end(), 16, ' ');
	// fill with: "abcdefghijklmnop"
	for (char c = 'a'; c < 'a' + 16; ++c)
		x[c - 'a'] = c;

	Gap_buffer::iterator iter = x.begin();
	const char* expected;
	// gap_position == 0 => data_begin == gap_begin && data_end != gap_end
	// gap_position == 1 => data_begin != gap_begin && data_end != gap_end
	// gap_position == 2 => data_begin != gap_begin && data_end == gap_end
	int gap_position = 2;
	switch (gap_position) {
	case 0:
		expected = "ijklmnop";
		break;
	case 1:
		for (int i = 0; i < 5; ++i)
			++iter;
		expected = "abcdenop";
		break;
	case 2:
		for (int i = 0; i < 8; ++i)
			++iter;
		expected = "abcdefgh";
		break;
	default:
		assert(0 && "Shouldn't get here");
	}
	x.erase(iter, 8);

	assert(*(x.begin() + 0) == x[0] && x[0] == expected[0]);
	assert(*(x.begin() + 1) == x[1] && x[1] == expected[1]);
	assert(*(x.begin() + 2) == x[2] && x[2] == expected[2]);
	assert(*(x.begin() + 3) == x[3] && x[3] == expected[3]);
	assert(*(x.begin() + 4) == x[4] && x[4] == expected[4]);
	assert(*(x.begin() + 5) == x[5] && x[5] == expected[5]);
	assert(*(x.begin() + 6) == x[6] && x[6] == expected[6]);
	assert(*(x.begin() + 7) == x[7] && x[7] == expected[7]);

	assert((x.begin() + 8) == x.end());

	assert(*(x.end() + -1) == x[7] && x[7] == expected[7]);
	assert(*(x.end() + -2) == x[6] && x[6] == expected[6]);
	assert(*(x.end() + -3) == x[5] && x[5] == expected[5]);
	assert(*(x.end() + -4) == x[4] && x[4] == expected[4]);
	assert(*(x.end() + -5) == x[3] && x[3] == expected[3]);
	assert(*(x.end() + -6) == x[2] && x[2] == expected[2]);
	assert(*(x.end() + -7) == x[1] && x[1] == expected[1]);
	assert(*(x.end() + -8) == x[0] && x[0] == expected[0]);

	assert(*(x.begin() - (-0)) == x[0] && x[0] == expected[0]);
	assert(*(x.begin() - (-1)) == x[1] && x[1] == expected[1]);
	assert(*(x.begin() - (-2)) == x[2] && x[2] == expected[2]);
	assert(*(x.begin() - (-3)) == x[3] && x[3] == expected[3]);
	assert(*(x.begin() - (-4)) == x[4] && x[4] == expected[4]);
	assert(*(x.begin() - (-5)) == x[5] && x[5] == expected[5]);
	assert(*(x.begin() - (-6)) == x[6] && x[6] == expected[6]);
	assert(*(x.begin() - (-7)) == x[7] && x[7] == expected[7]);

	assert((x.begin() - (-8)) == x.end());
	assert((x.end() - 0) == x.end());

	assert(*(x.end() - 1) == x[7] && x[7] == expected[7]);
	assert(*(x.end() - 2) == x[6] && x[6] == expected[6]);
	assert(*(x.end() - 3) == x[5] && x[5] == expected[5]);
	assert(*(x.end() - 4) == x[4] && x[4] == expected[4]);
	assert(*(x.end() - 5) == x[3] && x[3] == expected[3]);
	assert(*(x.end() - 6) == x[2] && x[2] == expected[2]);
	assert(*(x.end() - 7) == x[1] && x[1] == expected[1]);
	assert(*(x.end() - 8) == x[0] && x[0] == expected[0]);

	assert((x.begin() + 0) - x.begin() == 0);
	assert((x.begin() + 1) - x.begin() == 1);
	assert((x.begin() + 2) - x.begin() == 2);
	assert((x.begin() + 3) - x.begin() == 3);
	assert((x.begin() + 4) - x.begin() == 4);
	assert((x.begin() + 5) - x.begin() == 5);
	assert((x.begin() + 6) - x.begin() == 6);
	assert((x.begin() + 7) - x.begin() == 7);
	assert((x.begin() + 8) - x.begin() == 8);

	assert((x.begin() + 0) - x.end() == -8);
	assert((x.begin() + 1) - x.end() == -7);
	assert((x.begin() + 2) - x.end() == -6);
	assert((x.begin() + 3) - x.end() == -5);
	assert((x.begin() + 4) - x.end() == -4);
	assert((x.begin() + 5) - x.end() == -3);
	assert((x.begin() + 6) - x.end() == -2);
	assert((x.begin() + 7) - x.end() == -1);
	assert((x.begin() + 8) - x.end() == -0);

	assert((x.end() - 1) - (x.begin() + 1) == 6);
	assert((x.begin() + 1) - (x.end() - 1) == -6);
}
