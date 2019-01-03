#ifndef MY_BIT_MAP
#define MY_BIT_MAP

#include <set>

class MyBitMap {
	public:
		MyBitMap(int a, int b) { 
			counter = 0;
		}
		void setBit(int pos, int k) {
			if (k == 0)
				bits.insert(pos);
			else
				bits.erase(pos);
		}
		int findLeftOne() {
			int prev = 0;
			for (auto z : bits) {
				if (prev + 1 != z)
					return prev + 1;
				prev = z;
			}
			return prev + 1;
		}

	private:
		std::set<int> bits;
		int counter;
};

#endif
