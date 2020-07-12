#include "GameInfo.h"
#include "Ipn.h"
#include "Parser.h"

int main(int argc, char **argv)
{
	Parser parser;
	Ipn *ipn;
	FILE *f = fopen(argv[Script], "r");
	if (!f)
		return 0;
	if (!(ipn = parser.analyze(f)))
		return 0;
	try 
	{
		GameInfo gInfo(argc, argv);
		for(;;) {
			gInfo.UpdateInfo();	
			if (ipn->Perform(&gInfo))
				return 0;
			gInfo.WaitAuction();
		}
	}
	catch(const char *s) 
	{
		printf("%s\n", s);
	}
	delete ipn;
	return 0;
}
