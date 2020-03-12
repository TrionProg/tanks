
#include "OptionPtr.h"

const char * MyNullPointerException::what() const throw ()
{
	return "NullPointerException";
}