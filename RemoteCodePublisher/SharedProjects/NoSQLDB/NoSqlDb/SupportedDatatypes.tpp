
istream& operator >> (istream& in, string& ss) {
	ostringstream oss;
	string temp;
	while (in.good()) {
		getline(in, temp);
		oss << temp << "\n";
	}
	ss = oss.str().substr(0, oss.str().size() - 1);
	return in;
}

template <typename T>
istream& operator >> (istream& in, vector<T>& vec) {
	istringstream temp;
	string s;
	while (in.good()) {
		T val;
		getline(in, s);
		temp.str(s);
		while (temp.good()) {
			temp >> val;
			vec.push_back(val);
		}
		s.clear();
		temp.clear();
	}
	return in;
}

template <typename T>
ostream& operator << (ostream& out, vector<T>& vec) {
	int size = vec.size() - 1;
	for (int i = 0; i < size; i++) {
		out << vec[i] << " ";
	}
	out << vec[size] << "\n";
	return out;
}
