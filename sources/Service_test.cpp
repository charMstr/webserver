#include "../includes/Service_test.hpp"
CPPUNIT_TEST_SUITE_REGISTRATION(Service_test);

template <typename T>
void	printlist(std::list<T> & list)
{
	for (typename std::list<T>::iterator i = list.begin(); i != list.end(); ++i)
	{ std::cout << *i << " "; } std::cout << "\n";
}

void	Service_test::setUp()
{
	t_client_info client_info; client_info.fd = 1;
	t_error_page error_page;
	struct timeval dummy_timeout;

	// NB : fd_client gets automatically added to list_fd_write by Service's ctor
	_1_svc = new get_service(client_info, error_page, "", NULL, \
			"/autoindex/fake/uri", dummy_timeout);

	list_fd_init(_1_svc->list_fd_read, 2, 3, 4, 5);
	list_fd_init(_1_svc->list_fd_write, 6, 7);
}

void	Service_test::tearDown() {}

void	Service_test::test_remove_all_fds_in_list_fd_rw_except_for_fd_client()
{
	_1_svc->remove_all_fds_in_list_fd_rw_except_for_fd_client();
	/* list_fd_read should now be empty, and list_fd_write should only contain
	 * the fd_client */
	CPPUNIT_ASSERT(_1_svc->list_fd_read.empty()
					&& (_1_svc->list_fd_write.size() == 1)
					&& (_1_svc->list_fd_write.back() == _1_svc->client_info.fd));
}

void	Service_test::test_add_to_list_done_all_fds_except_for_fd_client()
{
	_1_svc->add_to_list_done_all_fds_except_for_fd_client();
	std::list<int>::iterator i;
	for (i = _1_svc->list_fd_read.begin(); i != _1_svc->list_fd_read.end(); ++i)
	{
		std::list<int>::iterator j = std::find(_1_svc->list_fd_done_reading.begin(),
										_1_svc->list_fd_done_reading.end(), *i);
		if (j != _1_svc->list_fd_done_reading.end()) { continue ; }
		else { break ; }
	}
	std::list<int>::iterator k;
	for (k = _1_svc->list_fd_write.begin(); k != _1_svc->list_fd_write.end(); ++k)
	{
		if (*k == _1_svc->client_info.fd) { continue ; }
		std::list<int>::iterator l = std::find(_1_svc->list_fd_done_writing.begin(),
										_1_svc->list_fd_done_writing.end(), *k);
		if (l != _1_svc->list_fd_done_writing.end()) { continue ; }
		else { break ; }
	}
	// also make sure we haven't copied the fd_client in list_done_writing
	std::list<int>::iterator m = std::find(_1_svc->list_fd_done_writing.begin(),
									_1_svc->list_fd_done_writing.end(),
									_1_svc->client_info.fd);
	/* all the fds present in list_fd_read and list_fd_write should also be
	 * present in list_fd_done_reading and list_fd_done_writing,
	 * except the fd_client which should not be in list_fd_done_writing */
	CPPUNIT_ASSERT(i == _1_svc->list_fd_read.end()
					&& k == _1_svc->list_fd_write.end()
					&& m == _1_svc->list_fd_done_writing.end());
}

/*
void	Service_test::test_read_error_from_fd_error_page()
{
	// read the error page into a buffer for comparison 
	char buf1[BUFFER_SIZE]; int fd1 = open("./www/error_default.html", O_RDONLY);
	int ret1 = read(fd1, &buf1, 99999); std::string s1(buf1, ret1);

	// save_fd_error_page because the methid read_error_page_from_fd_error_page
	// will reset it to -1 once its done reading
	int save_fd_error_page = _1_svc->fd_error_page;
	while (_1_svc->fd_error_page != -1)
		{ _1_svc->read_error_page_from_fd_error_page(); }

	std::list<int>::iterator i = std::find(_1_svc->list_fd_done_reading.begin(),
										_1_svc->list_fd_done_reading.end(),
										save_fd_error_page);

	// make sure that :
	// - error_page_body has the adequate contents
	// - list_fd_done_reading has fd_error_page
	///
	CPPUNIT_ASSERT(
			s1 == _1_svc->error_page_body
			&& i != _1_svc->list_fd_done_reading.end()
			);
	close(fd1);
}
*/
