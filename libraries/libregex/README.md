# libregex

## overview
Regex expressions are very useful to describe a formal language. We made a small library that converts a given regular expression into a language (represented by a DFA).
Then, several functions helps you to find occurrences of a valid sentence for this language in a bigger string, to check if a hole string match your expression, etc...

## public api

    t_regex		*re_from_str(const char *expr);
    int		re_match(t_regex *re, const char *str);
    char		*re_find(t_regex *re, const char*str);
    char		*re_rfind(t_regex *re, const char*str);

## supported regex operators
![Image](https://i.goopics.net/XJVlb.png)
