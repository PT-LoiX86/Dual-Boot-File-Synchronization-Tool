#ifndef CLI_H
#define CLI_H

#include "../../include/filesystem.h"
#include "../../include/backup.h"
#include "../../include/converter.h"

// ============ DISK / PARTITION CHECKING ============

int handle_disk_command();

// ============ FOLDER LINKING ============

int handle_link_command(int argc, char *argv[]);
int handle_unlink_command(int argc, char *argv[]);
int display_link_success(folder_link_t *link);
int display_link_already_exists(folder_link_t *existing_link, 
                                const char *provided_path, const char *other_path);
int display_unlink_confirmation(folder_link_t *link);
int display_verify_link_error(int error_code, folder_link_t *link);
int handle_links_list_command(int argc, char *argv[]);
int display_linked_folders(linked_folders_t *folders);

// ============ SYNC OPERATIONS ============

int display_sync_preview(sync_changes_t *changes, const char *source, const char *target);
int resolve_conflicts_interactive(sync_changes_t *changes, 
                                  conflict_resolution_t **resolutions, 
                                  int *resolution_count);
int display_final_confirmation(sync_changes_t *changes, int has_conflicts);
int display_sync_error_prompt(const char *filepath, const char *error_msg);
int handle_sync_command(int argc, char *argv[]);

// ============ BACKUP OPERATIONS ============

int display_backup_created(const char *backup_id, const char *backup_path, off_t size);
int display_restore_confirmation(const char *backup_id, const char *target_path);
int display_restore_successful(const char *backup_id, const char *target_path);
int display_backup_list(const char *link_id, backup_list_t *backup_list);
int display_cleanup_confirmation(const char *link_id, int backup_count);
int display_cleanup_successful(const char *link_id, int deleted_count);
int display_backup_error(const char *error_msg);
int display_auto_restore_start(const char *backup_id);
int display_auto_restore_successful(const char *backup_id);
int display_auto_restore_failed(const char *backup_id);

int handle_backups_list_command(int argc, char *argv[]);
int handle_backup_command(int argc, char *argv[]);
int handle_restore_command(int argc, char *argv[]);
int handle_backups_clean_command(int argc, char *argv[]);

// ============ LOGGING ============
int display_log_list(char **log_files, int log_count);
int display_log_list_since(char **log_files, int log_count, const char *date_str);
int display_log_tracking_start(void);
int display_log_tracking_end(void);
int display_log_latest(const char *log_path);

int handle_log_track_command(int argc, char *argv[]);
int handle_log_since_command(int argc, char *argv[]);
int handle_log_latest_command(int argc, char *argv[]);
int handle_log_list_command(int argc, char *argv[]);

// ============ CONVERTER ============
int display_conversion_prompt(const convertible_files_list_t *files_list);
int display_conversion_start(void);
int display_conversion_progress(int current_file, int total_files, 
                                const char *filename, const char *target_ext);
int display_conversion_app_not_found(const char *app_name, const char *filename);
int display_conversion_completed(int success_count, int fail_count, int skip_count);


#endif
