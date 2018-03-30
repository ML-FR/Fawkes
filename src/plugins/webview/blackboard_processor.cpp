
/***************************************************************************
 *  blackboard_processor.cpp - Web request processor for BlackBoard info
 *
 *  Created: Thu Oct 23 16:10:21 2008
 *  Copyright  2006-2008  Tim Niemueller [www.niemueller.de]
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "blackboard_processor.h"
#include <webview/page_reply.h>
#include <webview/file_reply.h>
#include <webview/error_reply.h>
#include <webview/url_manager.h>

#include <blackboard/blackboard.h>
#include <interface/interface.h>
#include <interface/field_iterator.h>
#include <interface/interface_info.h>
#include <utils/time/time.h>
#include <utils/misc/string_split.h>

#include <string>
#include <cstring>
#include <cstdlib>

#include <set>
#include <sstream>
#include <algorithm>
#include <functional>
#ifdef HAVE_GRAPHVIZ
#  include <gvc.h>
#  include <gvcjob.h>
#endif


using namespace fawkes;

/** @class WebviewBlackBoardRequestProcessor "blackboard_processor.h"
 * BlackBoard web request processor.
 * Provides access to BlackBoard introspection features.
 * @author Tim Niemueller
 */

/** Constructor.
 * @param url_manager URL manager to register with
 * @param blackboard BlackBoard instance
 */
WebviewBlackBoardRequestProcessor::WebviewBlackBoardRequestProcessor(fawkes::WebUrlManager *url_manager,
                                                                     BlackBoard *blackboard)
{
  blackboard_  = blackboard;
  url_manager_    = url_manager;

  url_manager_->add_handler(WebRequest::METHOD_GET, "/blackboard/view/{uid}",
                            std::bind(&WebviewBlackBoardRequestProcessor::process_view, this,
                                      std::placeholders::_1));
  url_manager_->add_handler(WebRequest::METHOD_GET, "/blackboard/graph/graph.png",
                            std::bind(&WebviewBlackBoardRequestProcessor::process_graph, this,
                                      std::placeholders::_1));
  url_manager_->add_handler(WebRequest::METHOD_GET, "/blackboard/graph/{for*}",
                            std::bind(&WebviewBlackBoardRequestProcessor::process_graphview, this,
                                      std::placeholders::_1));
  url_manager_->add_handler(WebRequest::METHOD_GET, "/blackboard/?",
                            std::bind(&WebviewBlackBoardRequestProcessor::process_overview, this));
}


/** Destructor. */
WebviewBlackBoardRequestProcessor::~WebviewBlackBoardRequestProcessor()
{
	url_manager_->remove_handler(WebRequest::METHOD_GET, "/blackboard/view/{uid}");
	url_manager_->remove_handler(WebRequest::METHOD_GET, "/blackboard/graph/graph.png");
	url_manager_->remove_handler(WebRequest::METHOD_GET, "/blackboard/graph/{for*}");
	url_manager_->remove_handler(WebRequest::METHOD_GET, "/blackboard");
  for (ifi_ = interfaces_.begin(); ifi_ != interfaces_.end(); ++ifi_) {
    blackboard_->close(ifi_->second);
  }
  interfaces_.clear();
}


WebReply *
WebviewBlackBoardRequestProcessor::process_graph(const fawkes::WebRequest *request)
{
#if defined(HAVE_GRAPHVIZ) && ((defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))) || defined(__clang__))
	std::string graph_node = request->get_value("for");
	std::string graph = generate_graph(graph_node);

	FILE *f = tmpfile();
	if (NULL == f) {
		return new WebErrorPageReply(WebReply::HTTP_INTERNAL_SERVER_ERROR,
		                             "Cannot open temp file: %s", strerror(errno));
	}

	GVC_t* gvc = gvContext(); 
	Agraph_t* G = agmemread((char *)graph.c_str());
	gvLayout(gvc, G, (char *)"dot");
	gvRender(gvc, G, (char *)"png", f);
	gvFreeLayout(gvc, G);
	agclose(G);    
	gvFreeContext(gvc);

	try {
		DynamicFileWebReply *freply = new DynamicFileWebReply(f);
		return freply;
	} catch (fawkes::Exception &e) {
		return new WebErrorPageReply(WebReply::HTTP_INTERNAL_SERVER_ERROR, *(e.begin()));
	}
#else
	return new WebErrorPageReply(WebReply::HTTP_INTERNAL_SERVER_ERROR,
	                             "BlackBoard processor was not built with Graphviz support");
#endif
}


WebReply *
WebviewBlackBoardRequestProcessor::process_graphview(const fawkes::WebRequest *request)
{
	std::string graph_node = request->path_arg("for");
	
	WebPageReply *r = new WebPageReply("BlackBoard");
	r->set_html_header("  <link type=\"text/css\" href=\""
	                   "/static/css/jqtheme/jquery-ui.custom.css\" rel=\"stylesheet\" />\n"
	                   "  <link type=\"text/css\" href=\""
	                   "/static/css/blackboard.css\" rel=\"stylesheet\" />\n");

#if defined(HAVE_GRAPHVIZ) && ((defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))) || defined(__clang__))
	std::string graph = generate_graph(graph_node);
	char *map;
	unsigned int map_length;

	GVC_t* gvc = gvContext(); 
	Agraph_t* G = agmemread((char *)graph.c_str());
	gvLayout(gvc, G, (char *)"dot");
	gvRenderData(gvc, G, (char *)"cmapx", &map, &map_length);
 	r->append_body("\n%s\n", map);
#if GRAPHVIZ_VERSION >= 23200
	gvFreeRenderData(map);
#else
	free(map);
#endif
	gvFreeLayout(gvc, G);
	agclose(G);    
	gvFreeContext(gvc);

 	r->append_body("<p><img src=\"/blackboard/graph/graph.png%s%s\" usemap=\"#bbmap\" /></p>\n",
		       graph_node.empty() ? "" : "?for=",
		       graph_node.empty() ? "" : graph_node.c_str());
	r->append_body("<!-- DOT Graph:\n\n%s\n\n-->\n\n", graph.c_str());
#else
	r->append_body("<p>No graphviz support at compile time</p>\n");
#endif
	if (! graph_node.empty()) {
	  r->append_body("<p><a href=\"/blackboard/graph\">Full Graph</a></p>\n\n");
	}
	return r;
}

WebReply *
WebviewBlackBoardRequestProcessor::process_view(const fawkes::WebRequest *request)
{
	std::string iuid = request->path_arg("uid");
	std::string iftype = iuid.substr(0, iuid.find("::"));
	std::string ifname = iuid.substr(iuid.find("::") + 2);

	WebPageReply *r = new WebPageReply("BlackBoard");
	r->set_html_header("  <link type=\"text/css\" href=\""
	                   "/static/css/jqtheme/jquery-ui.custom.css\" rel=\"stylesheet\" />\n"
	                   "  <link type=\"text/css\" href=\""
	                   "/static/css/blackboard.css\" rel=\"stylesheet\" />\n");

	r->append_body("<h2>Showing %s</h2>\n", iuid.c_str());
	if (interfaces_.find(iuid) == interfaces_.end()) {
		try {
			Interface *iface = blackboard_->open_for_reading(iftype.c_str(), ifname.c_str());
			interfaces_[iuid] = iface;
		} catch (Exception &e) {
			r->append_body("Failed to open interface: %s\n", e.what());
		}
	}
	if (interfaces_.find(iuid) != interfaces_.end()) {
		Interface *iface = interfaces_[iuid];
		iface->read();

		std::string writer;
		if (iface->has_writer()) {
			try {
				writer = iface->writer();
			} catch (Exception &e) {}
		}
		std::string readers;
		try {
			readers = str_join(iface->readers(), ", ");
		} catch (Exception &e) {}

		r->append_body("<table>\n"
		               " <tr><td><b>Type:</b></td><td>%s</td></tr>\n"
		               " <tr><td><b>ID:</b></td><td>%s</td></tr>\n"
		               " <tr><td><b>Writer:</b></td><td><span class=\"blackboard-writer-%s\">%s</span></td></tr>\n"
		               " <tr><td><b>Readers:</b></td><td>%s (%u)</td></tr>\n"
		               " <tr><td><b>Serial:</b></td><td>%u</td></tr>\n"
		               " <tr><td><b>Data size:</b></td><td>%u</td></tr>\n"
		               " <tr><td><b>Hash:</b></td><td>%s</td></tr>\n"
		               " <tr><td><b>Data changed:</b></td>"
		               "<td>%s (last at %s)</td></tr>\n"
		               "</table>\n",
		               iface->type(), iface->id(),
		               iface->has_writer() ? "exists" : "none",
		               iface->has_writer() ? writer.c_str() : "none",
		               iface->num_readers() > 0 ? readers.c_str() : "none",
		               iface->num_readers(),
		               iface->serial(),
		               iface->datasize(), iface->hash_printable(),
		               iface->changed() ? "yes" : "no", iface->timestamp()->str());

		r->append_body("<table>\n"
		               " <tr>\n"
		               "  <th>Name</th><th>Type</th><th>Value</th>\n"
		               " </tr>\n");
		for (InterfaceFieldIterator fi = iface->fields(); fi != iface->fields_end(); ++fi) {
			bool is_string = (fi.get_type() == IFT_STRING);
			*r += " <tr>\n";
			if ( fi.get_length() > 1 ) {
				r->append_body("  <td>%s</td><td>%s [%zu]</td><td>%s%s%s</td>\n",
				               fi.get_name(), fi.get_typename(),
				               fi.get_length(), is_string ? "<pre>" : "",
				               fi.get_value_string(), is_string ? "</pre>" : "");
			} else {
				r->append_body("  <td>%s</td><td>%s</td><td>%s%s%s</td>\n",
				               fi.get_name(), fi.get_typename(), is_string ? "<pre>" : "",
				               fi.get_value_string(), is_string ? "</pre>" : "");
			}
			*r += " </tr>\n";
		}
		r->append_body("</table>\n");
		r->append_body("<p><a href=\"/blackboard/\">Clear detailed</a></p>\n");
	}
	return r;
}

WebReply *
WebviewBlackBoardRequestProcessor::process_overview()
{
	WebPageReply *r = new WebPageReply("BlackBoard");
	r->set_html_header("  <link type=\"text/css\" href=\""
	                   "/static/css/jqtheme/jquery-ui.custom.css\" rel=\"stylesheet\" />\n"
	                   "  <link type=\"text/css\" href=\""
	                   "/static/css/blackboard.css\" rel=\"stylesheet\" />\n");


	*r += "\n\n<h2>Select Interface</h2>\n"
		"<div id=\"blackboard-interfaces-mainpart\">\n";

	bool found_some = false;
	InterfaceInfoList *iil = blackboard_->list_all();
	iil->sort();
	for (InterfaceInfoList::iterator i = iil->begin(); i != iil->end(); ++i) {
		if (! found_some) {
			*r += "<table>\n";
			*r += "<tr><th>Interface</th><th>Reader(s)</th><th>Writer</th></tr>\n";
			found_some = true;
		}
		r->append_body("<tr><td><a href=\"/blackboard/view/%s::%s\">%s::%s</a></td><td>%u</td><td style=\"color:%s\">%s</td></tr>\n",
		               i->type(), i->id(), i->type(), i->id(),
		               i->num_readers(), i->has_writer() ? "green" : "red", i->has_writer() ? i->writer().c_str() : "no");
	}
	delete iil;

	if (found_some) {
		*r += "</table>\n";
	}

#ifdef HAVE_GRAPHVIZ
	r->append_body("  <div class=\"blackboard-graph-div\">"
	               "<a href=\"/blackboard/graph\" class=\"blackboard-graph-link\">Graph</a></div>\n");
#endif

	*r += "  </div>\n";

	if (! found_some) {
		*r += "<p><b>No interfaces found.</b></p>\n";
	}

	return r;
}

#if defined(HAVE_GRAPHVIZ) && ((defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))) || defined(__clang__))
std::string
WebviewBlackBoardRequestProcessor::generate_graph(std::string for_owner)
{
  InterfaceInfoList *iil = blackboard_->list_all();
  iil->sort();

  std::stringstream mstream;
  mstream << "digraph bbmap {" << std::endl
	  << "  graph [fontsize=12,rankdir=LR];" << std::endl;

  std::set<std::string> owners;

  InterfaceInfoList::iterator ii;
  for (ii = iil->begin(); ii != iil->end(); ++ii) {
    const std::list<std::string> readers = ii->readers();
    
    if (for_owner == "" ||
	ii->writer() == for_owner ||
	std::find_if(readers.begin(), readers.end(),
		     [&for_owner](const std::string &o)->bool { return for_owner == o; })
	!= readers.end())
    {
      if (ii->has_writer()) {
	const std::string writer = ii->writer();
	if (! writer.empty())  owners.insert(writer);
      }
      std::list<std::string>::const_iterator r;
      for (r = readers.begin(); r != readers.end(); ++r) {
	owners.insert(*r);
      }
    }
  }

  mstream << "  node [fontsize=12 shape=box width=4 margin=0.05];" << std::endl
	  << "  { rank=same; " << std::endl;
  std::set<std::string>::iterator i;
  for (ii = iil->begin(); ii != iil->end(); ++ii) {
    const std::list<std::string> readers = ii->readers();
    if (for_owner == "" ||
	ii->writer() == for_owner ||
	std::find_if(readers.begin(), readers.end(),
		     [&for_owner](const std::string &o)->bool { return for_owner == o; })
	!= readers.end())
    {
      mstream << "    \"" << ii->type() << "::" << ii->id() << "\""
	      << " [href=\"/blackboard/view/" << ii->type() << "::" << ii->id() << "\"";


      if (! ii->has_writer()) {
	mstream << " color=red";
      } else if (ii->writer().empty()) {
	mstream << " color=purple";
      }
      mstream << "];" << std::endl;
    }
  }
  mstream << "  }" << std::endl;

  mstream << "  node [fontsize=12 shape=octagon width=3];" << std::endl;
  for (i = owners.begin(); i != owners.end(); ++i) {
    mstream << "  \"" << *i << "\""
	    << " [href=\"/blackboard/graph/" << *i << "\"];"
	    << std::endl;
  }

  for (ii = iil->begin(); ii != iil->end(); ++ii) {
    const std::list<std::string> readers = ii->readers();
    if (for_owner == "" ||
	ii->writer() == for_owner ||
	std::find_if(readers.begin(), readers.end(),
		     [&for_owner](const std::string &o)->bool { return for_owner == o; })
	!= readers.end())
    {
      std::list<std::string> quoted_readers;
      std::for_each(readers.begin(), readers.end(),
		    [&quoted_readers](const std::string &r) {
		      quoted_readers.push_back(std::string("\"")+r+"\"");
		    });
      std::string quoted_readers_s = str_join(quoted_readers, ' ');
      mstream << "  \"" << ii->type() << "::" << ii->id() << "\" -> { "
	      << quoted_readers_s << " } [style=dashed arrowhead=dot arrowsize=0.5 dir=both];" << std::endl;

      if (ii->has_writer()) {
	mstream << "  \"" << (ii->writer().empty() ? "???" : ii->writer()) << "\" -> \""
		<< ii->type() << "::" << ii->id() << "\""
		<< (ii->writer().empty() ? " [color=purple]" : " [color=\"#008800\"]")
		<< ";" << std::endl;
      }
    }
  }

  delete iil;

  mstream << "}";
  return mstream.str();
}
#endif
