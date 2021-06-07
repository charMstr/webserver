#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <time.h>
# include <sstream>
# include <stdio.h>
# include <sys/time.h>
# include <sys/stat.h>

#include "webserver.hpp"

unsigned long	ft_stoul(const std::string& str, std::size_t* pos = 0, int base = 10);
std::string		ft_ultos(unsigned long n);
char			*ft_strdup(const char *src);

//make it possible to reuse a socket quickly.
void	set_socket_rebindable_for_quick_restart(int sock);

//sets the O_NONBLOCK flags on a fd.
void	set_nonblock(int socket);

//sets the O_NONBLOCK flags on a fd.
void			set_nonblock(int socket);

std::string		get_the_time(void);
void			get_chunk(std::string & payload, std::string & data, size_t & already_encoded_data);
int				concatenate_path(const std::string &root, std::string &ressource);
std::string		locate_ressource(const std::vector<t_loc> &locations, const std::string &default_root, const std::string& path);
std::string		get_ip(const struct sockaddr_in &addr);
bool			do_file_exists(const std::string &path);
std::string		get_last_modified_time(const struct stat * fileStat);
void			get_chunk(std::string & payload,
							std::string & data, size_t & already_encoded_data);
char			*ft_strnstr(const char *haystack, const char *needle, size_t len);
int				predict_chunk_tag_space(size_t space_left_in_buffer);
std::string		get_close_chunk_string(int size_chunk);
int				ci_find_substr(const std::string &str1, const std::string &str2);
const t_loc		*get_location_block_to_use(const std::string &uri, t_srv &server);

int				ft_stoi(std::string s);

#endif // UTILS_HPP
