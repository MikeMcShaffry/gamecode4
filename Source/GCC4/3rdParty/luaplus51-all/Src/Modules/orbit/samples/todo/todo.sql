CREATE TABLE todo_list
       ("id" INTEGER PRIMARY KEY NOT NULL,
       "title" VARCHAR(255) DEFAULT "",
       "done" BOOLEAN DEFAULT "f",
       "created_at" DATETIME);
