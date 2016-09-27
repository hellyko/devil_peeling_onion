#include <iostream>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>

class Picture;
class String_pic;
class Frame_pic;
class Vcat_pic;
class Hcat_pic;

Picture frame (const Picture&, const char& = '*');



//generic Handle with ref count

template <typename T>
class Handle_gen
{
	public:
	//construct
	Handle_gen(): base_ptr(0), base_count(new size_t(1)){}
	Handle_gen(T* base): base_ptr(base), base_count(new size_t(1)){}
	//copy assign delete
	Handle_gen(const Handle_gen& base): base_ptr(base.base_ptr), base_count(base.base_count)
	{
		++*base.base_count;
	}
	Handle_gen& operator=(const Handle_gen& base)
	{
		++*base.base_count;
		--*base_count;
		if (*base_count == 0)
		{
			delete base_count;
			delete base_ptr;
		}
		base_ptr = base.base_ptr;
		base_count = base.base_count;
		return *this;
	}
	~Handle_gen()
	{
		--*base_count;
		if (*base_count == 0)
		{
			delete base_count;
			delete base_ptr;
		}
	}

	//utility
	operator bool() const
	{
		return base_ptr;
	}
	T& operator*()
	{
		if (base_ptr) return *base_ptr;
		else throw std::runtime_error("unbound Handle");
	}
	T* operator->() const
	{
		if (base_ptr) return base_ptr;
		else throw std::runtime_error("unbound Handle");
	}
	protected:
	private:
	T* base_ptr;
	size_t* base_count;
};

//fully abstract class
class Pic
{	
	public:
	virtual ~Pic() = 0;
	friend class Picture;
	friend class Frame_pic;
	friend class Vcat_pic;
	friend class Hcat_pic;
	friend class String_pic;
	friend std::ostream& operator << (std::ostream&, const Picture&);
	private:
	typedef std::string::size_type h_size;
	typedef std::vector<std::string>::size_type v_size;
	virtual	h_size width() const = 0;
	virtual v_size height() const = 0;
	virtual void display(std::ostream&, v_size, bool) const = 0;
	protected:
	static std::ostream& pad(std::ostream& out, h_size first, h_size last)
	{
		while (first < last)
		{
			out << ' ';	
			first++;
		}
		return out;
	}
};

Pic::~Pic()
{}

//#########################################################################
//picture made from strings only class that holds actual data

class String_pic: public Pic
{
	friend class Picture;
	public:
	~String_pic(){}
	protected:
	private:
	String_pic(const std::vector<std::string>& base): base_vec(base){}
	//utility
	h_size width() const
	{
		std::string::size_type val = 0;
		for (v_size i = 0; i < height(); i++)
		{
			val = std::max(val, base_vec[i].size());
		}
		return val;
	}
	v_size height() const
	{
		return base_vec.size();
	}
	void display(std::ostream& out, v_size val_v, bool padding) const
	{
		h_size val_h = 0;
		if (val_v < height())
		{
			out << base_vec[val_v];
			val_h = base_vec[val_v].size();
			if (padding)
			pad(out, val_h, width());
		}
		else if (padding)
		pad(out, 0, width());
		return;
	}
	//data
	std::vector<std::string> base_vec;
};

//#########################################################################
//picture with frame done based on string_pic no public data - all in Picture class

class Frame_pic: public Pic
{
	friend Picture frame(const Picture&, const char&);
	public:
	~Frame_pic(){}
	protected:
	private:
	//construct
	Frame_pic(const Handle_gen<Pic>& base, const char& border = '*'): str_pic_base(base), base_border(border) {}
	//utility
	v_size height() const
	{
		return (str_pic_base -> height()) + 4;
	}
	h_size width() const
	{
		return (str_pic_base -> width()) + 4;
	}
	void display(std::ostream& out, v_size val_v, bool padding) const
	{
		if (val_v == 0)
		out << std::string(width(), base_border);
		else if (val_v == 1)
		{
			out << base_border;
			out << std::string(width() - 2, ' ');
			out << base_border;
		}
		else if (val_v < height() - 2)
		{
			out << base_border << ' ';
			str_pic_base -> display(out, val_v - 2, true);
			out << ' ' << base_border;
		}
		else if (val_v == height() - 2)
		{
			out << base_border;
			out << std::string(width() - 2, ' ');
			out << base_border;
		}
		else if (val_v == height() - 1)
		out << std::string(width(), base_border);
		else if (padding)
		{
			str_pic_base -> display(out, val_v, true);
			out << std::string(4, ' ');
		}
		return;
	}
	//data
	Handle_gen<Pic> str_pic_base;
	char base_border;
};

//#########################################################################
//result of horisontal concatenation

class Hcat_pic: public Pic
{
	friend Picture hcat(const Picture&, const Picture&);
	public:
	~Hcat_pic(){}
	protected:
	private:
	Hcat_pic(const Handle_gen<Pic>& left, const Handle_gen<Pic>& right): first(left), second(right){}
	//utility
	h_size width() const
	{
		return first -> width() + second -> width();
	}

	v_size height() const
	{
		return std::max(first -> height(), second -> height());	
	}

	void display(std::ostream& out, v_size val_v, bool padding) const
	{	
		first -> display(out, val_v, true);
		second -> display(out, val_v, padding);	
		return;
	}
	//data
	Handle_gen<Pic> first;
	Handle_gen<Pic> second;
};

//#########################################################################
//result of vertical concatenation
class Vcat_pic: public Pic
{
	friend Picture vcat(const Picture&, const Picture&);
	public:
	~Vcat_pic(){}
	protected:
	private:
	Vcat_pic(const Handle_gen<Pic>& top, const Handle_gen<Pic>& bottom): first(top), second(bottom) {}
	//utility
	h_size width() const
	{
		return std::max(first -> width(), second -> width());
	}

	v_size height() const
	{
		return first -> height() + second -> height();
	}

	void display(std::ostream& out, v_size val_v, bool padding) const
	{
		h_size pad_h = 0;
		if (val_v < first -> height())
		{
			first -> display(out, val_v, padding);
			pad_h = first -> width();
		}
		else
		{
			second -> display(out, val_v - first -> height(), padding);
			pad_h = second -> width();
		}
		if (padding)
		{	
			pad(out, pad_h, width());
		}
		return;			
	}
	//data
	Handle_gen<Pic> first;
	Handle_gen<Pic> second;
};

//#########################################################################
//wrap around class. mostly interface.

class Picture
{
	friend Picture frame(const Picture&, const char&);
	friend Picture hcat(const Picture&, const Picture&);
	friend Picture vcat(const Picture&, const Picture&);
	friend std::ostream& operator << (std::ostream&, const Picture&); 
	public:
	Picture(const std::vector<std::string>& base = std::vector<std::string>()): base_pic(new String_pic(base)) {}
	protected:
	private:
	//private constructors
	Picture(Pic* base): base_pic(base){}
	Handle_gen<Pic> base_pic;
};

//each of these interface funcs should create an obj of Children family. Then it should wrap it with Picture obj.
Picture frame(const Picture& base, const char& border)
{		
	return new Frame_pic(base.base_pic, border);	
}
Picture hcat(const Picture& first, const Picture& second)
{
	return new Hcat_pic(first.base_pic, second.base_pic);
}
Picture vcat(const Picture& first, const Picture& second)
{
	return new Vcat_pic(first.base_pic, second.base_pic);
}
std::ostream& operator <<(std::ostream& out, const Picture& base)
{
	const Pic::v_size pic_height = base.base_pic -> height();
	for (Pic::v_size i = 0; i < pic_height; i++)
	{
		base.base_pic -> display(out, i, false);
		out << std::endl;
	}
	return out;
}

int main()
{
	std::vector<std::string> data;
	std::string record;
	char border = '*';
	char tmp_c;
	std::string tmp_str;
	std::cout << "Border character to use?" << std::endl;
	std::getline(std::cin, tmp_str);
	std::stringstream tmp_ss(tmp_str);
	tmp_ss >> border;
	tmp_ss >> std::ws;
	if (tmp_ss >> tmp_c) throw std::invalid_argument("Invalid input stream still valid");
	if (isspace(border)) border = '*';
	std::cout << "Add picture" << std::endl;
	do 
	{
		if (std::cin)
		{
			record.clear();
			std::getline(std::cin, record);
			if (!record.empty())
			{
				data.push_back(record);
			}
		}
	}
	while (std::cin && !record.empty());

	Picture pic(data);
	std::cout << pic << std::endl;
	Picture q = frame(pic, border);
	std::cout << q << std::endl;
	Picture r = hcat(pic, q);
	std::cout << r << std::endl;
	Picture t = vcat(r, q);
	std::cout << t << std::endl;
	std::cout << frame(hcat(t, vcat(r, q))) << std::endl;
	return 0;
}

