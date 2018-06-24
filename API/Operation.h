#include "../Interpreter/llvmsql.h"
#include "Result.h"
#include "../RecordManager/record.h"
#include "llvm/ADT/STLExtras.h"
#include "../catalog/catalog.h"
#include <sstream>

using namespace std;

class Operation
{
public:

protected:
	std::shared_ptr<StatementAST> stat;
};
class Api
{
public:
	static void operate(std::shared_ptr<StatementAST> stat);
};
class Create:Operation
{
public:
	/*
	1 CreateTableSimple 
	2 CreateTableSelect 
	3 CreateTableLike
	11 CreateIndex
	*/
	int create_kind;
	Create(std::shared_ptr<StatementAST> stat);
	Result create_table();
	Result create_table_simple();
	Result create_table_select();
	Result create_table_like();
	Result create_index();
};
class Select:Operation
{
public:
	Result select();
	Select(std::shared_ptr<StatementAST> stat);
	bool dele;
	static int judge_int;
	static float judge_float;
	static char* judge_string;
	static vector<Column> get_cols(string table);

	static bool gteq_int(char* s)
	{
		int v = ((int *)s)[0];
		return v >= Select::judge_int;
	}
	static bool gt_int(char* s)
	{
		int v = ((int *)s)[0];
		return v > Select::judge_int;
	}
	static bool lteq_int(char* s)
	{
		int v = ((int *)s)[0];
		return v <= Select::judge_int;
	}
	static bool lt_int(char* s)
	{
		int v = ((int *)s)[0];
		return v < Select::judge_int;
	}
	static bool eq_int(char* s)
	{
		int v = ((int *)s)[0];
		return v == Select::judge_int;
	}
	static bool not_eq_int(char* s)
	{
		int v = ((int *)s)[0];
		return v != Select::judge_int;
	}

private:
	
	int select_kind;
	std::shared_ptr<TableNameAST> tbname;
	vector<string> fields;

	bool distinct_flag = false;
	bool from_flag = false;
	bool where_flag = false;
	bool group_flag = false;
	bool having_flag = false;
	bool order_flag = false;

	Result ex_expr(std::shared_ptr<ExprAST> expr);
	Result solve_predicate(std::shared_ptr<PredicateAST> predicate);
	Result solve_bp(std::shared_ptr<BooleanPrimaryAST> bp);
	Result select_all();
	void add_fields(std::vector<std::shared_ptr<SelectExprAST>> exprs);
	
};

class Drop:Operation
{
public:
	//1 drop table 2 drop index
	int drop_kind;
	std::vector<std::shared_ptr<IdAST>> table_list;
	std::vector<std::shared_ptr<IdAST>> index_list;
	Drop(std::shared_ptr<StatementAST> stat);
	static void drop_cata(string tbname);
	Result drop();
};
class Insert:Operation
{
public:
	std::shared_ptr<IdAST> table_name;
	std::vector<std::shared_ptr<IdAST>> col_names;
	std::vector<std::shared_ptr<ExprAST>> value_list;
	Insert(std::shared_ptr<StatementAST> stat);
	Result insert();

private:
};
class Dele:Operation
{
public:
	Result dele();
	Dele(std::shared_ptr<StatementAST> stat);
private:
	
};
class Setvar:Operation
{
public:
	shared_ptr<IdAST> id;
	shared_ptr<ExprAST> expr;
	Setvar(std::shared_ptr<StatementAST> stat);
};

bool gteq_int(char* s);
bool gteq_float(char* s);
bool gt_int(char* s);
bool gt_float(char* s);
bool lteq_int(char* s);
bool lteq_float(char* s);
bool lt_int(char* s);
bool lt_float(char* s);
bool eq_int(char* s);
bool eq_float(char* s);
bool not_eq_int(char* s);
bool not_eq_float(char* s);
bool eq_string(char* v);
bool not_eq_string(char* v);
bool al_true(char* v);