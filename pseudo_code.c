z* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pseudo_code.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/13 13:45:41 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/15 11:09:26 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

int	main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	1) creer un objet Server de type "server" avec en parametre le vecteur qu'a
		cree loic.
		--> si la creation fail, throw une exception.
		--> si la creation fonctionne: l'objet Server contient un autre vector
		associant une paire: [fd_liste, virtual_server]


	2) Server.start_work()
		--> la boucle avec select: voir plus bas.

	3) Server.shutdown()
		--> close tous les fd_listen, cleen memory
		--> potentiellement arriver la quand il y a eu un signal de type ctrl^C
		et gerer la memoire en accord...

	return (0);
}

//class server, il manque tout sauf la fonction qui utilise select()
class server
{
	//method ou on a la boucle principal de select().
	int start_work()
	{
		declaration des fd_set que select() ne modifiera pas, 3 tableaux:
			fd_set_read_real, fd_set_write_real et fd_set_except_real;
		while(1)
		{
			remplir fd_set_read_copy;
			remplir fd_set_write_copy;
			remplir fd_set_except_copy;

			appeler select();

			note: ne regarder que le fd_set_read_copy pour comprendre au debut.

			1) si dans le tableau fd_set_read_copy on a un fd qui correspond au
				fd_listen d'un element de la liste chainee pairs_fd_listen_t_srv
				--> on sait que la socket d'ecoute (fd_listen) peut accepter une
				nouvelle connexion.
				--> appeler accept: fd_client = accept(fd_listen, ...);
				--> On ajoute le nouveau fd_client au tableau fd_set_read_real,
					il sera surveille par select dorenavant.
				--> creer un nouveau maillon de type pair_List_fd_Service et l'
				ajouter a la liste des services en cours de traitement. Ce
				maillon contient:
					- le fd_client place dans une liste de fd, qui sert a
					toujours retrouver le service en cours.
					- un pointer sur un objet derive de la class abstraite
					Service. Allouer ce pointer avec new. Dans notre cas l'
					object derive est de la class Request:
					- l'objet request comprend un buffer dans lequel on lit.
					- ptr_server_block (qui provient de la liste chainee de
						virtual_server_block_&&_fd_pair) qui va nous permettre
						de valider ou non la request quand on l'a entierrement
						recue.

			2) parcourir la liste de services en cours (pair_List_fd_Service),
			et a chaque maillon, checker si les fd sont soit dans:
				- un fd_set_read (appeler la methode read de l'objet concret).
				- un fd_set_write (appeler la methode write...).

			3) si on arrive a la fin de vie d'un service, il faudra detruire
			l'objet en question (le retirer de la liste de services en cours),
			et bien retirer ses fd des fd_sets (real pas copy). apres les avoir
			close().
				note: si l'objet est de type request, il faut regarder si la
					requete est valide, si oui, eventuellement creer un objet
					derive de type service et l'ajouter...

		}
	}
}
