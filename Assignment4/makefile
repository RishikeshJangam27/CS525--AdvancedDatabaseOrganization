default: test_assign4 test_expr

test_assign4: test_assign4_1.o btree_mgr.o rm_serializer.o record_mgr.o dberror.o storage_mgr.o buffer_mgr.o  expr.o
	gcc test_assign4_1.o btree_mgr.o rm_serializer.o record_mgr.o dberror.o storage_mgr.o buffer_mgr.o  expr.o -o test_assign4 -lm

test_expr: test_expr.o btree_mgr.o rm_serializer.o record_mgr.o dberror.o storage_mgr.o buffer_mgr.o  expr.o
	gcc test_expr.o btree_mgr.o rm_serializer.o record_mgr.o dberror.o storage_mgr.o buffer_mgr.o  expr.o -o test_expr -lm

test_expr.o: test_expr.c dberror.h expr.h record_mgr.h tables.h test_helper.h btree_mgr.h
	gcc -c test_expr.c

test_assign4_1.o: test_assign4_1.c btree_mgr.h dberror.h expr.h record_mgr.h tables.h test_helper.h btree_mgr.h
	gcc -c test_assign4_1.c

rm_serializer.o: dberror.h record_mgr.h tables.h
	gcc -c rm_serializer.c

record_mgr.o: record_mgr.c dberror.h storage_mgr.h buffer_mgr.h record_mgr.h tables.h  expr.h
	gcc -c record_mgr.c

expr.o: expr.c dberror.h expr.h tables.h record_mgr.h
	gcc -c expr.c

buffer_mgr_stat.o: buffer_mgr_stat.c buffer_mgr_stat.h buffer_mgr.h
	gcc -c buffer_mgr_stat.c

buffer_mgr.o: buffer_mgr.c buffer_mgr.h storage_mgr.h dberror.h dt.h
	gcc -c buffer_mgr.c

btree_mgr.o: btree_mgr.c btree_mgr.h buffer_mgr.h storage_mgr.h dberror.h dt.h
	gcc -c btree_mgr.c

storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h
	gcc -c storage_mgr.c 

dberror.o: dberror.c dberror.h
	gcc -c dberror.c

clean:
	rm test_assign4 test_expr *.o

run:
	./test_assign4
