#include <stdlib.h>
#include <iup.h>

int main(int argc, char **argv)
{
	IupOpen(&argc, &argv);
	IupMessage("Hello World Example", "Hello World from IUP.");
	IupClose();
	return EXIT_SUCCESS;
}
