#ifndef SET_H
#define SET_H

/**
 * set_write - write config file including changes to stdout
 */
void set_write(const char *filename);

int set_key_exists(const char *key);

/**
 * set_set - set/add value in config file
 *
 * @is_commented_out: start line with '#'
 * If key already exists, it will be overwritten.
 * If it does not exist, it will be added.
 */
void set_set(const char *key, const char *value, int is_commented_out);

int set_is_already_set_correctly(const char *key, const char *value);

/**
 * set_read - read config file
 *
 * This is needed before setting/adding key/value pairs
 */
void set_read(const char *filename);

#endif /* SET_H */
