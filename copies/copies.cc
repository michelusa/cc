#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
	vector<int> v = {1, 2, 3, 4, 5, 6};

	copy(begin(v), end(v), ostream_iterator<int>(cout, " "));
	cout << endl;

	vector<int> w(v.size(), 999);
	cout << "backward first three: ";
	
	copy_backward(begin(v), begin(v) + 3, end(w));
	copy(begin(w), end(w), ostream_iterator<int>(cout, " "));
	cout << endl;
	
	vector<int> rw(v.size(), 999);
	cout << "reverse first three :";
	
	reverse_copy(begin(v), begin (v) + 3, begin(rw));
	copy(begin(rw), end(rw), ostream_iterator<int>(cout, " "));
	cout << endl;
}
