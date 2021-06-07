#ifndef SERVICE_TEST_HPP
# define SERVICE_TEST_HPP

# include <cppunit/extensions/HelperMacros.h>
# include <strstream>
# include "../includes/Service.hpp"
# include "../includes/get_service.hpp"

class Service_test : public CppUnit::TestFixture
{
	friend class Service;
	public:
		void setUp();
		void tearDown();

	private:
		template <typename ...Ints>
		void list_fd_init(std::list<int> & list, Ints... ints)
		{
			int args[] { ints... }; // unpack
			size_t size = sizeof(args) / sizeof(args[0]);
			for (int i = 0; i < size; ++i)
			{
				list.push_back(args[i]);
			}
		}
		/* initialise the t_srv block needed by the concrete class Error_service ctor */
		void initialise_t_error_page(t_error_page &error_p, const char *str, \
										int err1, int err2, int err3) {
			error_p.error_page = std::string(str);
			if (err1 >= 300 && err1 <= 599)
				error_p.list_error_status.push_back(err1);
			if (err2 >= 300 && err2 <= 599)
				error_p.list_error_status.push_back(err2);
			if (err3 >= 300 && err3 <= 599)
				error_p.list_error_status.push_back(err3);
		}
		void serv_block_init(t_srv &server, bool set_index_page, bool set_error_page,
						const char *name_error_page, int err1, int err2, int err3)
		{
			if (set_index_page)
				server.index = std::string("index.html");
			if (set_error_page)
				initialise_t_error_page(server.error_page, name_error_page, err1, err2, err3);
		}

		/* Service class specimens */
		Service * _1_svc;
		/* needed t_srv for construction */
		t_srv _no_index_no_error_no_location;

		// Tests:
		void	test_remove_all_fds_in_list_fd_rw_except_for_fd_client();
		void	test_add_to_list_done_all_fds_except_for_fd_client();

		CPPUNIT_TEST_SUITE(Service_test);

		CPPUNIT_TEST(test_remove_all_fds_in_list_fd_rw_except_for_fd_client);
		CPPUNIT_TEST(test_add_to_list_done_all_fds_except_for_fd_client);

		CPPUNIT_TEST_SUITE_END();

};

#endif // SERVICE_TEST_HPk
