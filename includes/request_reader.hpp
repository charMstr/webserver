#ifndef REQUEST_READER_HPP
#define REQUEST_READER_HPP

#include "request_parser.hpp"
#include "webserver.hpp"
#include <sys/socket.h>

#define HEADERS_MAX_SIZE 2048

/*
**	Request_reader reads a request from a client (on a socket) and parses
**	it in a most usable way.
**	It proceeds step by step, splitting the work into several parts:
**	Request line, leading headers, body and trailing headers.
**	Each part is proccessed once it is entirely read.
*/
class Service;

class Request_reader
{
	//DEBUG && TEST
	friend class Cgi_service_test;
	friend void	debug_request_reader(const Request_reader &reader);

	public:

		// Internal types
		enum request_state
		{
			REQUEST_LINE,
			HEADERS,
			BODY,
			DONE,
		};

		// Constructors
		Request_reader(const t_client_info &client, const t_srv &server);
		Request_reader(Request_reader const &src);
		Request_reader &operator=(Request_reader const &rhs);

		// Destructor
		virtual	~Request_reader(void);

		// Member functions
		int								read_on_client_socket();
		bool							write_no_more_alloc_answer(const char *msg, const size_t msg_size);
		void							dump_request_buffer(void);
		void							set_up_related_to_cgi(t_loc const *location_block_to_use);
		void 							flush_reader_for_keep_alive(void);

		// Getters
		bool							is_done_reading() const;
		bool							can_be_deleted() const;
		int								get_client_fd() const;
		const t_client_info				&get_client_infos() const;
		const struct sockaddr_storage	&get_client_addr() const;
		size_t							get_client_addr_size() const;
		const t_srv						&get_server() const;
		int								get_status(void) const;
		int								get_method(void) const;
		const std::string				&get_uri(void) const;
		const Headers					&get_headers() const;
		const std::string				get_header(std::string) const;
		bool							is_a_cgi_request(void) const;
		const std::string				&get_query_string(void) const;
		const std::string				&get_mime_type() const;
		const std::string				&get_cgi_bin_name() const;
		size_t							get_content_length(void) const;
		size_t							get_cli_max_size(void) const;
		request_state					get_request_state(void) const;
		bool							does_request_timed_out(void);
		bool			get_object_is_now_no_more_alloc_handler(void) const;
		//makes sure the reader can still keep alive. false == last service
		bool				get_request_counter_for_keep_alive_done() const
		{
			if (request_counter_for_keep_alive <= 0)
				return (true);
			return (false);
		}

		// Setters
		void							request_counter_for_keep_alive_minus_one()
		{
			request_counter_for_keep_alive--;	
		}

		void							reset_timeout_reference(void);
		void			set_object_is_now_no_more_alloc_handler(void);
		//called when an error occured in classes containing reader.
		void							set_state_to_done(void);
		void							reset_reader(void);

	private:

		// Member functions
		int								request_line_handler(void);
		int								headers_handler(void);
		int								body_handler(void);

		// Setters
		void							set_body_attributes(void);
		void							set_cli_max_size(void);

		// Attributes
		t_client_info					client;
		t_srv							server;
		//this is used to know if the reader can be deleted once the service is fully done.
	public:
		bool							ill_formed_request_so_no_more_reading;
		bool							service_already_generated;
		bool							i_can_be_deleted;
		Service							*ptr_service_generated;
		//reader added to any service created. It might be set to NULL.
		std::string						body_from_client;
	private:
		//counts the number of requests exchanged between client and reader
		//since ever. it dimishes until zero.
		int								request_counter_for_keep_alive;
		char							buffer[BUFFER_SIZE];
		size_t							recv_calls;
		size_t							current_content_length;
		size_t							cli_max_size;
	public:
		std::string						request_buffer;
	private:
		request_state					state;
		Request_parser					parser;
		int								status;
		bool							object_is_now_no_more_alloc_handler_bool;
		size_t							position_in_no_more_alloc_answer;
	public:
		struct timeval					timeout_reference;
};

#endif /* REQUEST_READER_HPP */
