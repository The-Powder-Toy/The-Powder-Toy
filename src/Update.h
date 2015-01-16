#ifndef UPDATE_H_
#define UPDATE_H_

//char *exe_name(void);
int update_start(char *data, unsigned int len);
int update_finish(void);
void update_cleanup(void);

#endif /* UPDATE_H_ */
