#pragma once

class sub_token
{
public:
	sub_token()
	{
		_value = next();
	}
	sub_token(const sub_token& other)
	{
		_value = other._value;
	}

	void operator=(const sub_token& other)
	{
		_value = other._value;
	}

	void disconnect()
	{
		//_subject->unsubscribe(*this);
	}

	bool operator==(const sub_token& other) const
	{
		if (other == *this)
			return false;
		return this->_value == other._value;
	}

	bool operator<(const sub_token& other) const
	{
		return this->_value < other._value;
	}

	int value() { return _value; }

private:
	int next() { return ++count; }
	int _value;
	static int count;
};
