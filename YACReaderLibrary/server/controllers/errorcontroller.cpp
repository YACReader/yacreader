#include "errorcontroller.h"

#include "template.h"
#include "../static.h"


ErrorController::ErrorController(int errorCode)
:error(errorCode)
{}

void ErrorController::service(HttpRequest& request, HttpResponse& response)
{
	switch(error)
	{
	case 300:
		response.setStatus(300,"redirect");
		response.write("<html> <head> <meta http-equiv=\"refresh\" content=\"0; URL=/\"> </head> <body> </body> </html>", true);
		break;
	case 404:
		response.setStatus(404,"not found");
		response.write("404 not found",true);
		break;
	}

}