#include "nothydrus.h"

sqlite3_stmt* add_file_statement;
sqlite3_stmt* filepath_from_id_statement;
sqlite3_stmt* flags_from_id_statement;
sqlite3_stmt* filesize_from_id_statement;
sqlite3_stmt* hash_from_id_statement;
sqlite3_stmt* id_from_filepath_statement;
sqlite3_stmt* set_file_flags_statement;
sqlite3_stmt* add_flag_to_file_statement;
sqlite3_stmt* remove_flag_from_file_statement;
sqlite3_stmt* add_tag_statement;
sqlite3_stmt* add_taggroup_statement;
sqlite3_stmt* tag_id_from_name_statement;
sqlite3_stmt* taggroup_id_from_name_statement;
sqlite3_stmt* tag_name_from_id_statement;
sqlite3_stmt* tag_fullname_from_id_statement;
sqlite3_stmt* taggroup_name_from_id_statement;
sqlite3_stmt* tag_statement;
sqlite3_stmt* untag_statement;
sqlite3_stmt* search_tags_statement;
sqlite3_stmt* get_file_tags_statement;
sqlite3_stmt* add_custom_column_statement;
sqlite3_stmt* get_file_columns_statement;
sqlite3_stmt* search_file_from_hash_statement;
sqlite3_stmt* set_filepath_statement;

unsigned int custom_columns_n;
struct custom_column* custom_columns;

static inline void prepare_add_file(){
	if(sqlite3_prepare_v3(main_db,
				"INSERT INTO files("
				"hash, size, filetype, flags, filepath) "
				"VALUES(?, ?, ?, ?, ?);"
				, -1, SQLITE_PREPARE_PERSISTENT, &add_file_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing add_file_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_filepath_from_id(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT filepath FROM files "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &filepath_from_id_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing filepath_from_id_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_flags_from_id(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT flags FROM files "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &flags_from_id_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing flags_from_id_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_filesize_from_id(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT size FROM files "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &filesize_from_id_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing filesize_from_id_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_hash_from_id(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT hash FROM files "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &hash_from_id_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing hash_from_id_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_id_from_filepath(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT id FROM files "
				"WHERE filepath = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &id_from_filepath_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing id_from_filepath_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_set_file_flags(){
	if(sqlite3_prepare_v3(main_db,
				"UPDATE files "
				"SET flags = ? "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &set_file_flags_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing set_file_flags_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_add_flag_to_file(){
	if(sqlite3_prepare_v3(main_db,
				"UPDATE files "
				"SET flags = flags | ? "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &add_flag_to_file_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing add_flag_to_file_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_remove_flag_from_file(){
	if(sqlite3_prepare_v3(main_db,
				"UPDATE files "
				"SET flags = flags & ~? "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &remove_flag_from_file_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing remove_flag_from_file_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_add_tag(){
	if(sqlite3_prepare_v3(main_db,
				"INSERT INTO tags("
				"name, taggroup) "
				"VALUES(?, ?);"
				, -1, SQLITE_PREPARE_PERSISTENT, &add_tag_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing add_tag_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_add_taggroup(){
	if(sqlite3_prepare_v3(main_db,
				"INSERT INTO taggroups(name) "
				"VALUES(?);"
				, -1, SQLITE_PREPARE_PERSISTENT, &add_taggroup_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing add_taggroup_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_tag_id_from_name(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT id FROM tags "
				"WHERE name = ? AND taggroup = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &tag_id_from_name_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing tag_id_from_name_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_tag_name_from_id(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT name, taggroup FROM tags "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &tag_name_from_id_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing tag_name_from_id_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_tag_fullname_from_id(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT CASE tags.taggroup "
					"WHEN 1 THEN tags.name "
					"ELSE taggroups.name||':'||tags.name "
				"END "
				"FROM tags "
				"JOIN taggroups ON tags.taggroup = taggroups.id "
				"AND tags.id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &tag_fullname_from_id_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing tag_fullname_from_id_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_taggroup_name_from_id(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT name FROM taggroups "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &taggroup_name_from_id_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing taggroup_name_from_id_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_taggroup_id_from_name(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT id FROM taggroups "
				"WHERE name = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &taggroup_id_from_name_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing taggroup_id_from_name_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_tag(){
	if(sqlite3_prepare_v3(main_db,
				"INSERT INTO filestags(file,tag) "
				"VALUES(?,?);"
				, -1, SQLITE_PREPARE_PERSISTENT, &tag_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing tag_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_untag(){
	if(sqlite3_prepare_v3(main_db,
				"DELETE FROM filestags "
				"WHERE file = ? AND tag = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &untag_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing untag_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_search_tags(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT tags.id FROM tags JOIN taggroups ON taggroups.id=tags.taggroup AND taggroups.name LIKE ? "
				"WHERE tags.name LIKE ? "
				"ORDER BY tags.number;"
				, -1, SQLITE_PREPARE_PERSISTENT, &search_tags_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing search_tags_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_get_file_tags(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT tag FROM filestags "
				"WHERE file = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &get_file_tags_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing get_file_tags_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

void prepare_add_custom_column(){
	if(sqlite3_prepare_v3(main_db,
				"INSERT INTO custom_columns("
				"name, type, flags, lower_limit, upper_limit) "
				"VALUES(?, ?, ?, ?, ?);"
				, -1, SQLITE_PREPARE_PERSISTENT, &add_custom_column_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing add_custom_column_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_get_file_columns(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT * FROM files "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &get_file_columns_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing get_file_columns_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_search_file_from_hash(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT id FROM files "
				"WHERE size = ? AND hash = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &search_file_from_hash_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing search_file_from_hash_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static inline void prepare_set_filepath(){
	if(sqlite3_prepare_v3(main_db,
				"UPDATE files "
				"SET filepath = ? "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &set_filepath_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing set_filepath_statement: %s\n", sqlite3_errmsg(main_db));
	}
}


void start_program(int_least8_t flags){
	chdir(main_path);
	if(sqlite3_open(INIT_DIRECTORY"/"MAIN_DATABASE_NAME, &main_db)){
		fprintf(stderr, "Error opening main database: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	if(flags & START_PROGRAM_INIT){
		return;
	}
	get_custom_columns();
	if(flags & START_PROGRAM_ADD_FILES){
		prepare_add_file();
		return;
	}
	if(flags & START_PROGRAM_SQL_SEARCH){
		prepare_filepath_from_id();
		prepare_flags_from_id();
		return;
	}
	if(flags & START_PROGRAM_TAG){
		prepare_id_from_filepath();
		prepare_add_tag();
		prepare_add_taggroup();
		prepare_tag_id_from_name();
		prepare_taggroup_id_from_name();
		prepare_tag();
		return;
	}
	prepare_add_file();
	prepare_filepath_from_id();
	prepare_flags_from_id();
	prepare_filesize_from_id();
	prepare_hash_from_id();
	prepare_id_from_filepath();
	prepare_set_file_flags();
	prepare_add_flag_to_file();
	prepare_remove_flag_from_file();
	prepare_add_tag();
	prepare_add_taggroup();
	prepare_tag_id_from_name();
	prepare_taggroup_id_from_name();
	prepare_tag_name_from_id();
	prepare_tag_fullname_from_id();
	prepare_taggroup_name_from_id();
	prepare_tag();
	prepare_untag();
	prepare_search_tags();
	prepare_get_file_tags();
	prepare_add_custom_column();
	prepare_get_file_columns();
	prepare_search_file_from_hash();
	prepare_set_filepath();
}

void end_program(){
	sqlite3_finalize(add_file_statement);
	sqlite3_finalize(filepath_from_id_statement);
	sqlite3_finalize(flags_from_id_statement);
	sqlite3_finalize(filesize_from_id_statement);
	sqlite3_finalize(hash_from_id_statement);
	sqlite3_finalize(id_from_filepath_statement);
	sqlite3_finalize(set_file_flags_statement);
	sqlite3_finalize(add_flag_to_file_statement);
	sqlite3_finalize(remove_flag_from_file_statement);
	sqlite3_finalize(add_tag_statement);
	sqlite3_finalize(add_taggroup_statement);
	sqlite3_finalize(tag_id_from_name_statement);
	sqlite3_finalize(taggroup_id_from_name_statement);
	sqlite3_finalize(tag_name_from_id_statement);
	sqlite3_finalize(tag_fullname_from_id_statement);
	sqlite3_finalize(taggroup_name_from_id_statement);
	sqlite3_finalize(tag_statement);
	sqlite3_finalize(untag_statement);
	sqlite3_finalize(search_tags_statement);
	sqlite3_finalize(get_file_tags_statement);
	sqlite3_finalize(add_custom_column_statement);
	sqlite3_finalize(get_file_columns_statement);
	sqlite3_finalize(search_file_from_hash_statement);
	sqlite3_finalize(set_filepath_statement);

	sqlite3_close(main_db);
}
