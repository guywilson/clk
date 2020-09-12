#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <logger.h>
#include <mongoose.h>
#include <html_template.h>

extern "C" {
#include <strutils.h>
}

#include "views.h"
#include "webadmin.h"
#include "clk_error.h"

extern "C" {
#include "version.h"
}

using namespace std;

static char * getMethod(struct http_message * message)
{
	char *				pszMethod;

	pszMethod = (char *)malloc(message->method.len + 1);

	if (pszMethod == NULL) {
		throw clk_error(clk_error::buildMsg("Failed to allocate %d bytes for method...", message->method.len + 1), __FILE__, __LINE__);
	}

	memcpy(pszMethod, message->method.p, message->method.len);
	pszMethod[message->method.len] = 0;

	return pszMethod;
}

static char * getURI(struct http_message * message)
{
	char *				pszURI;

	pszURI = (char *)malloc(message->uri.len + 1);

	if (pszURI == NULL) {
		throw clk_error(clk_error::buildMsg("Failed to allocate %d bytes for URI...", message->uri.len + 1), __FILE__, __LINE__);
	}

	memcpy(pszURI, message->uri.p, message->uri.len);
	pszURI[message->uri.len] = 0;

	return pszURI;
}

static struct mg_serve_http_opts getHTMLOpts()
{
	static struct mg_serve_http_opts opts;

	WebAdmin & web = WebAdmin::getInstance();

	opts.document_root = web.getHTMLDocRoot();
	opts.enable_directory_listing = "no";

	return opts;
}

static struct mg_serve_http_opts getCSSOpts()
{
	static struct mg_serve_http_opts opts;

	WebAdmin & web = WebAdmin::getInstance();

	opts.document_root = web.getCSSDocRoot();
	opts.enable_directory_listing = "no";

	return opts;
}

void cmdHandler(struct mg_connection * connection, int event, void * p)
{
	struct http_message *			message;
	char *							pszMethod;
	char *							pszURI;
	char 							szCmdValue[64];
	const char *					pszRedirect = "/cmd/cmd.html";
	int								rtn;

	Logger & log = Logger::getInstance();

	switch (event) {
		case MG_EV_HTTP_REQUEST:
			message = (struct http_message *)p;

			pszMethod = getMethod(message);
			pszURI = getURI(message);

			log.logInfo("Got %s request for '%s'", pszMethod, pszURI);

			if (strncmp(pszMethod, "GET", 4) == 0) {
				rtn = mg_get_http_var(&message->query_string, "command", szCmdValue, 32);

				if (rtn < 0) {
					throw clk_error(clk_error::buildMsg("Failed to find form variable 'command' for URI %s", pszURI), __FILE__, __LINE__);
				}

				log.logInfo("Got command: %s", szCmdValue);

				if (strncmp(szCmdValue, "debug-logging-on", 16) == 0) {
					int level = log.getLogLevel();
					level |= (LOG_LEVEL_INFO | LOG_LEVEL_DEBUG);
					log.setLogLevel(level);
					pszRedirect = "/cmd/cmdon.html";
				}
				else if (strncmp(szCmdValue, "debug-logging-off", 17) == 0) {
					int level = log.getLogLevel();
					level &= ~(LOG_LEVEL_INFO | LOG_LEVEL_DEBUG);
					log.setLogLevel(level);
					pszRedirect = "/cmd/cmd.html";
				}
			}

			mg_http_send_redirect(
							connection, 
							302, 
							mg_mk_str(pszRedirect), 
							mg_mk_str(NULL));

			free(pszMethod);
			free(pszURI);

			connection->flags |= MG_F_SEND_AND_CLOSE;
			break;

		default:
			break;
	}
}

void viewHandler(struct mg_connection * connection, int event, void * p)
{
	struct http_message *			message;
	char *							pszMethod;
	char *							pszURI;
	char *							pszCLKVersion;

	Logger & log = Logger::getInstance();

	switch (event) {
		case MG_EV_HTTP_REQUEST:
			message = (struct http_message *)p;

			pszMethod = getMethod(message);
			pszURI = getURI(message);

			log.logInfo("Got %s request for '%s'", pszMethod, pszURI);
	
			if (strncmp(pszMethod, "GET", 3) == 0) {
				WebAdmin & web = WebAdmin::getInstance();

				log.logInfo("Serving file '%s'", pszURI);

				if (str_endswith(pszURI, "/") > 0) {
					string htmlFileName(web.getHTMLDocRoot());
					htmlFileName.append(pszURI);
					htmlFileName.append("index.html");

					string templateFileName(htmlFileName);
					templateFileName.append(".template");

					log.logDebug("Opening template file [%s]", templateFileName.c_str());

					tmpl::html_template templ(templateFileName);

					templ("clk-version") = pszCLKVersion;

					templ.Process();

					fstream fs;
					fs.open(htmlFileName, ios::out);
					fs << templ;
					fs.close();
				}

				mg_serve_http(connection, message, getHTMLOpts());
			}

			free(pszMethod);
			free(pszURI);
			break;

		default:
			break;
	}
}

void cssHandler(struct mg_connection * connection, int event, void * p)
{
	struct http_message *			message;
	char *							pszMethod;
	char *							pszURI;

	Logger & log = Logger::getInstance();

	switch (event) {
		case MG_EV_HTTP_REQUEST:
			message = (struct http_message *)p;

			pszMethod = getMethod(message);
			pszURI = getURI(message);

			log.logInfo("Got %s request for '%s'", pszMethod, pszURI);

			if (strncmp(pszMethod, "GET", 3) == 0) {
				log.logInfo("Serving file '%s'", pszURI);

				mg_serve_http(connection, message, getCSSOpts());
			}

			free(pszMethod);
			free(pszURI);

			mg_printf(connection, "HTTP/1.1 200 OK");
			connection->flags |= MG_F_SEND_AND_CLOSE;
			break;

		default:
			break;
	}
}
