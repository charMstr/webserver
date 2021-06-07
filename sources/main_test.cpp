/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_test.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 10:36:04 by lfalkau           #+#    #+#             */
/*   Updated: 2021/04/23 12:37:27 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/XmlOutputter.h>
#define OUTPUT_XML_FILE "./tests_logs/output.xml"

std::map<std::string, std::string> mime_types;

void	produce_output_xml(CppUnit::TextTestRunner const &runner)
{
	CppUnit::XmlOutputter *xml_outputter = NULL;
	std::ofstream xml_out(OUTPUT_XML_FILE);
	xml_outputter = new CppUnit::XmlOutputter(&runner.result(), xml_out);
	xml_outputter->write();
	xml_out.close();	
	delete xml_outputter;
}

int main(void)
{
	CppUnit::TextTestRunner runner;

	// make sure all the files have macro CPPUNIT_TEST_SUITE_REGISTRATION() and
	// it is outside the class difinition.
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());

	runner.run();

	//uncomment to produce the output.xml file.
//	produce_output_xml(runner); 

	return (!runner.result().wasSuccessful()); //0 means ok.
}
