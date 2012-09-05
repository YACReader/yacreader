/**
  @file
  @author Stefan Frings
*/

#include "formcontroller.h"
#include <QStringList>

FormController::FormController() {}

void FormController::service(HttpRequest& request, HttpResponse& response) {

    response.setHeader("Content-Type", "text/html; charset=utf-8");

	QString data(request.getBody());

	QStringList list = data.split("\n");

	response.write("<html><body>");
	response.writeText("á é í ó ú ñ -> \\ /Device type: "+list.first());

	//test background proccesing
	/*int i=0;
	int j=0;
	while(i<1000000000)
	{
		if(request.getBody().length()>1)
			j++;
		else
			i++;
		if(i%1000000 == 0)
			response.write("<p> lista </p>");
	}*/

	response.write("<p> lista </p>");

	response.write("<ul>");

	for(int i=1;i<list.length();i++)
	{
		response.writeText("<li>"+list.at(i)+"</li>");
	}
	response.write("</ul></body></html>",true);
    
	/*if (request.getParameter("action")=="show") {
        response.write("<html><body>");
        response.write("Name = ");
        response.write(request.getParameter("name"));
        response.write("<br>City = ");
        response.write(request.getParameter("city"));
        response.write("</body></html>",true);
    }
    else {
        response.write("<html><body>");
        response.write("<form method=\"post\">");
        response.write("  <input type=\"hidden\" name=\"action\" value=\"show\">");
        response.write("  Name: <input type=\"text\" name=\"name\"><br>");
        response.write("  City: <input type=\"text\" name=\"city\"><br>");
        response.write("  <input type=\"submit\">");
        response.write("</form>");
        response.write("</body></html>",true);
    }*/
}

