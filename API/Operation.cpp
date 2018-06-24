#include "Operation.h"

using namespace std;

void Api::operate(std::shared_ptr<StatementAST> stat)
{
	if (!stat->create) {
		Create create(stat);
		create.create_table();
	}
	else if (!stat->select) {
		Select select(stat);
		select.select();
	}
	else if (!stat->drop) {
		Drop drop(stat);
		drop.drop();
	}
	else if (!stat->insert) {
		Insert insert(stat);
		insert.insert();
	}
	else if (!stat->dele) {
		Dele dele(stat);
		dele.dele();
	}
	else if (!stat->setvar) {

	}
}


int exec_file()
{
	/**********TODO*/
	string filename;
	return 0;
}
Create::Create(std::shared_ptr<StatementAST> stat)
{
	Create::stat = stat;
	if (stat->create->cindex) {
		Create::create_kind = 11;
	}
	else if (stat->create->ctable){
		if (stat->create->ctable->simplecreate) {
			Create::create_kind = 1;
		}
		else if (stat->create->ctable->selectcreate) {
			Create::create_kind = 2;
		}
		else if (stat->create->ctable->likecreate) {
			Create::create_kind = 3;
		}
		else
		{
			//error
		}
	}
	else
	{
		//error
	}
}
Result Create::create_table()
{
	switch (Create::create_kind) {
	case 1:
		return create_table_simple();
		break;
	case 2:
		return create_table_select();
		break;
	case 3:
		return create_table_like();
		break;
	case 4:
		return create_index();
	default:;
	}

	return Result(1);
}
Result Create::create_table_simple()
{
	vector<Column> columns = vector<Column>();
	std::shared_ptr<IdAST> table_name = stat->create->ctable->simplecreate->table_name;
	std::vector<std::shared_ptr<CreatedefAST>> create_defs = stat->create->ctable->simplecreate->create_defs;
	for (std::shared_ptr<CreatedefAST> create_def : create_defs) {
		/*解析参数*/
		std::shared_ptr<IdAST> colname = create_def->colname;
		std::shared_ptr<ColdefAST> coldef = create_def->coldef;
		DataType type;
		DataRule rule;
		{
			std::shared_ptr<DatatypeAST> dtype = coldef->dtype;
			switch (dtype->dtype) {
			case tok_INT:
				type = DB_INT;
				break;
			case tok_FLOAT:
				type = DB_FLOAT;
				break;
			case tok_STRING:
				type = DB_STRING;
				break;
			default:
				break;
			}
			bool null_flag = coldef->null_flag;
			if (null_flag) {
				rule = NONE;
			}
			std::shared_ptr<ExprAST> default_value = coldef->default_value;
			bool unic_flag = coldef->unic_flag;
			if (unic_flag) {
				rule = rule = UNIQUE;
			}
			bool primary_flag = coldef->primary_flag;
			if (primary_flag) {
				rule = PRIMARY_KEY;
			}
			std::shared_ptr<RefdefAST> refdef;
			{
				std::shared_ptr<IdAST> tbname = refdef->tbname;
				std::vector<std::shared_ptr<IdAST>> colname = refdef->colname;
				int deleteop = refdef->deleteop;
				int updateop = refdef->updateop;
			}
		}
		std::vector<std::shared_ptr<IdAST>> prim_cols = create_def->prim->cols;
		std::vector<std::shared_ptr<IdAST>> uni_cols = create_def->uni->cols;
		std::shared_ptr<ForeignAST> forei = create_def->forei;
		{
			std::vector<std::shared_ptr<IdAST>> cols = forei->cols;
			std::shared_ptr<RefdefAST> refdef = forei->refdef;
			{
				std::shared_ptr<IdAST> tbname = refdef->tbname;
				std::vector<std::shared_ptr<IdAST>> colname = refdef->colname;
				int deleteop = refdef->deleteop;
				int updateop = refdef->updateop;
			}
		}
		std::shared_ptr<ExprAST> check = create_def->check;
		Column column = make_tuple(
			(*colname->id).c_str(),
			type,
			30,
			rule
			);
		columns.push_back(column);
	}

	Table((unsigned int)catalog::catamap[*(table_name->id)], 
		(*(table_name->id)).c_str(),columns);

	return Result(1);
}
Result Create::create_table_select()
{
	return Result();
}
Result Create::create_table_like()
{
	return Result();
}
Result Create::create_index()
{
	return Result();
}

Result Select::ex_expr(std::shared_ptr<ExprAST> expr)
{
	/*
	最顶层的expr是带逻辑与 或的
	exp就是带一个否定的
	*/
	Result result;
	if (!expr->rhs) {//rhs为空，只有lhs exp
		if (!expr->lhs->expr) {//只有bp，解析bp
			//bp:null 和比较的是boolean primary，，，比如 5+9*2>-6
			std::shared_ptr<BooleanPrimaryAST> bp = expr->lhs->bp;
			result = Select::solve_bp(bp);
		}
		else {
			//
		}
	}
	else {
		//todo
	}
	return result;
}
/* bp解析得到的结果：
	1 predicate ，用于bp中的 op操作(构造语法中一个bp由pred，op，bp组成，
	事实上可以改为两个pred和一个op)
	2 op操作的结果，即得到的vector
*/
Result Select::solve_bp(std::shared_ptr<BooleanPrimaryAST> bp)
{
	int op = bp->op;
	//如果flag是tok_NULL 或者 tok_NOT,就说明这个bp是
	//由 boolean_primary IS [NOT] NULL构成的，否则flag就应该是0，否则就抛异常
	int flag = bp->flag;
	//单个predicate的解析
	//简单起见不考虑predicate的其他成员，只考虑其中的bitexpr，即下面else中的
	std::shared_ptr<PredicateAST> predicate1 = bp->p;
	Result result1 = Select::solve_predicate(predicate1);
	if (flag == tok_NULL || tok_NOT) {
		//
	}
	else if (flag != 0) {
		//exception
	}
	//然后看op，如果op也是0就说明这个bp是由1个predicate构成的
	if (op != 0) {
		std::shared_ptr<PredicateAST> predicate2 = solve_bp(bp->bp).predicate;
		Result result2 = Select::solve_predicate(predicate2);
		string col1, col2, tab1, tab2;
		vector<Column> cols1, cols2;
		vector<map<string, BaseData*>> res;

		//得到Result1的表名
		if (result1.type == res_COLUMN) {
			col1 = *(result1.id->id);
			tab1 = *tbname->tbname->id;
		}
		if (result1.type == res_TABLE_COL) {
			col1 = *result1.tablecol->col_name;
			tab1 = *result1.tablecol->table_name;
		}
		if (result2.type == res_COLUMN || result2.type == res_TABLE_COL) {
			if (result2.type == res_COLUMN) {
				col2 = *result2.id->id;
				tab2 = *tbname->tbname->id;
			}
			else {
				col2 = *result2.tablecol->col_name;
				tab2 = *result2.tablecol->table_name;
			}
			//两列比较
		}
		else
		{
			//构造table用以查询
			cols1 = Select::get_cols(tab1);
			const char* a = strdup(tab1.c_str());//string转换char*

												 //数值比较
			if (result2.type == tok_INT ) {

				switch (op)
				{
				case eq_mark:
					if (dele == false) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), eq_int);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), eq_int);
					}
					break;
				case gteq_mark:
					if (dele == false) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), gteq_int);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), gteq_int);
					}
					break;
				case gt_mark:
					if (dele == false) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), gt_int);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), gt_int);
					}
					break;
				case lteq_mark:
					if (dele == false) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res =t->query(Select::fields, string(a), lteq_int);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), lteq_int);
					}
					break;
				case lt_mark:
					if (dele == false) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), lt_int);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), lt_int);
					}
					break;
				case ltgt_mark:
					//
					break;
				case noteq_mark:
					if (dele == false) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), not_eq_int);

					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), not_eq_int);
					}
					break;
				default:
					break;
				}
			}
			if (result2.type == tok_FLOAT) {
				switch (op)
				{
				case eq_mark:
					if (dele == true) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), eq_float);

					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), eq_float);

					}
					break;
				case gteq_mark:
					if (dele == true) {
						Table *t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), gteq_float);
					}
					else {
						Table *t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), gteq_float);
					}
					break;
				case gt_mark:
					if (dele == true) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), gt_float);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), gt_float);
					}
					break;
				case lteq_mark:
					if (dele == true){
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), lteq_float);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), lteq_float);
					}
					break;
				case lt_mark:
					if (dele == true) {
						Table* t= new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), lt_float);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res =t->query(Select::fields, string(a), lt_float);
					}
					break;
				case ltgt_mark:
					//
					break;
				case noteq_mark:
					if (dele == true) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), not_eq_float);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), not_eq_float);
					}
					break;
				default:
					break;
				}
			}
			if (result2.type == tok_STRING) {
				switch (op)
				{
				case eq_mark:
					if (dele == true) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), eq_string);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), eq_string);
					}
					break;
				case gteq_mark:
					break;
				case gt_mark:
					break;
				case lteq_mark:
					break;
				case lt_mark:
					break;
				case ltgt_mark:
					break;
				case noteq_mark:
					if (dele == true) {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						t->deleteRecord(string(a), not_eq_string);
					}
					else {
						Table* t = new Table(catalog::catamap[tab1], a, cols1);
						res = t->query(Select::fields, string(a), not_eq_string);
					}
					break;
				default:
					break;
				}
			}
		}
		if (res.empty())
			return Result(1);
		else
			return Result(res);
	}
	else
	{//op == 0,代表只包含一个predicate
		return Result(bp->p ,*bp->p);
	}
	if (!bp->sub) {
		//暂时不考虑
	}
}

Result Select::select_all()
{
	vector<map<string, BaseData*>> res;
	const string tab = *(Select::tbname->tbname->id);
	int id = catalog::catamap[tab];
	if (dele == false) {
		Table* t = new Table(id, strdup(tab.c_str()), get_cols(tab));
		res =t ->query(Select::fields, fields[0], al_true);
	}
	else {
		Table* t = new Table(id, strdup(tab.c_str()), get_cols(tab));
		t->deleteRecord(fields[0], al_true);
	}
	return Result();
}

void Select::add_fields(std::vector<std::shared_ptr<SelectExprAST>> exprs)
{
	for(std::shared_ptr<SelectExprAST> expr : exprs)
		Select::fields.push_back(*(expr->expr->lhs->bp->p->bitexpr->bitexp->bitex->SE->id->id));
}

vector<Column> Select::get_cols(string table)
{
	unsigned int tab_id;

	vector<Column> cols = vector<Column>();

	if (catalog::catamap.count(table) > 0) {
		tab_id = catalog::catamap[table];
	}

	string file = catalog::cata_path + std::to_string(tab_id)+ ".log";
	std::ifstream r(file);

	char buff[256];
	int cnt;
	r.getline(buff , 100);//name
	r.getline(buff , 100);//行数
	string s = string(buff);
	stringstream stream(s);
	stream  >> cnt;
	r.getline(buff, 100);//
	int i = 0;
	for (; i < cnt; i++) {
		int col_para[5];
		r.getline(buff, 100);
		const char *sep = " "; //可按多个字符来分割
		char *p;
		p = strtok(buff, sep);
		char* col_name = p;
		p = strtok(NULL, sep);
		
		for (int j = 0; j < 5; j++) {
			string s = string(p);
			stringstream stream(s);
			stream >> col_para[j];
			p = strtok(NULL, sep);
		}
		DataType type;
		switch (col_para[0])
		{
		case tok_INT:
			type = DB_INT;
			break;
		case tok_FLOAT:
			type = DB_FLOAT;
			break;
		case tok_STRING:
			type = DB_STRING;
			break;
		default:
			break;
		}
		DataRule rule;
		if (col_para[3]) {
			rule = PRIMARY_KEY;
		}
		if (col_para[4]) {
			rule = UNIQUE;
		}
		if (col_para[5]) {
			rule = NONE;
		}
		Column col =  make_tuple(p, type, 30, rule);
		cols.push_back(col);
	}
	return cols;
}

Result Select::solve_predicate(std::shared_ptr<PredicateAST> predicate)
{
	//predicate先不用考虑太复杂，就当做简单的simple expr
	bool pred_flag = predicate->flag;
	if (predicate->sub) {
		//bit_expr [NOT] IN (subquery)
		std::shared_ptr<SubqueryAST> sub = predicate->sub;

	}
	else if (!predicate->exprs.empty()) {
		//bit_expr [NOT] IN (expr[])
		std::vector<std::shared_ptr<ExprAST>> exprs = predicate->exprs;

	}
	else if (!predicate->p && predicate->p) {
		//bit_expr [NOT] BETWEEN bit_expr AND predicate
		std::shared_ptr<PredicateAST> p = predicate->p;
		std::shared_ptr<BitExprAST> rhs = predicate->rhs;
	}
	else
	{
		//bitexpr
		std::shared_ptr<BitExprAST> bitexpr = predicate->bitexpr;
		//bitexpr的解析
		std::shared_ptr<BitExpAST> bitexp = bitexpr->bitexp;
		if (bitexpr->bitexpr) {
			//二元加减操作

		}
		else {
			//乘除操作
			//bitex的解析
			std::shared_ptr<BitExAST> bitex = bitexp->bitex;

			if (bitexp->bitexp) {
				//二元乘除
				std::shared_ptr<BitExpAST> bitexp;
			}
			else {
				if (bitex->mark != 0) {
				}
				else {
					std::shared_ptr<SimpleExprAST> se = bitex->SE;

					//id代表列名
					std::shared_ptr<IdAST> id = se->id;
					if (id) {
						return Result(id, id->id);
					}
					//TablecolAST代表  表名.列名
					std::shared_ptr<TablecolAST> tablecol = se->tablecol;
					if (tablecol) {
						return Result(tablecol, *tablecol);
					}
					std::shared_ptr<LiteralAST> lit = se->lit;
					if (lit){
						if (lit->intvalue) {
							return Result(*lit->intvalue->value);
						}
						if (lit->doublevalue) {
							return Result(*lit->intvalue->value);
						}
						if (lit->stringvalue) {
							return Result(*lit->intvalue->value);
						}
					}

					//暂时不考虑
					if (se->call) {

					}
					if (se->expr) {

					}
					if (se->sub) {

					}
					if (se->exists) {

					}
				}
			}
			int op;
		}
	}
	return Result();
}

Result Select::select()
{
	Result result;
	if (Select::distinct_flag == true) {
	}

	std::vector<std::shared_ptr<SelectExprAST>> exprs = stat->select->subquery->exprs;
	Select::add_fields(exprs);

	if (Select::from_flag == true) {
		std::shared_ptr<TableRefsAST> tbrefs = stat->select->subquery->tbrefs; {
			std::vector<std::shared_ptr<TableRefAST>> refs = tbrefs->refs;
			//recursive
			for(std::shared_ptr<TableRefAST> ref : refs) {
				std::shared_ptr<TableFactorAST> tbfactor = ref->tbfactor;
				{
					//直接获取表名
					Select::tbname = tbfactor->tbname;
					std::shared_ptr<TableQueryAST> tbsub = tbfactor->tbsub;
					std::shared_ptr<TableRefsAST> tbrefs = tbfactor->tbrefs;
				}
				std::shared_ptr<TRIJAST> trij = ref->trij;
				{
					std::shared_ptr<TableRefAST> ref = trij->ref;
					//recursive
					std::shared_ptr<TableFactorAST> factor = trij->factor;
					std::shared_ptr<JoinCondAST> cond = trij->cond;	// optional
				}
				std::shared_ptr<TRLROJAST> trlroj = ref->trlroj;
				std::shared_ptr<TRNLROJAST> trnlroj = ref->trnlroj;
			}
		}
	}
	if (Select::where_flag == true) {
		std::shared_ptr<ExprAST> wherecond = stat->select->subquery->wherecond;
		//如果select操作 得到的是带结果的vector
		//如果是delete操作，得到Result(1)
		result = ex_expr(wherecond);
	}
	else {
		result = Select::select_all();
	}
	if (Select::group_flag == true) {
		std::vector<std::shared_ptr<ExprAST>> groupby_exprs = stat->select->subquery->groupby_exprs;
	}
	if (Select::having_flag == true) {
		std::shared_ptr<ExprAST> havingcond = stat->select->subquery->havingcond;
	}
	if (Select::order_flag == true) {
		std::vector<std::shared_ptr<ExprAST>> orderby_exprs = stat->select->subquery->orderby_exprs;
	}
	return result;
}
Select::Select(std::shared_ptr<StatementAST> stat)
{
	Select::stat = stat;
	if (!stat->select->subquery) {
		if (stat->select->subquery->distinct_flag == true) {
			Select::distinct_flag = true;
		}
		if (stat->select->subquery->from_flag == true) {
			Select::from_flag = true;
		}
		if (stat->select->subquery->where_flag == true) {
			Select::where_flag = true;
		}
		if (stat->select->subquery->group_flag == true) {
			Select::group_flag = true;
		}
		if (stat->select->subquery->having_flag == true) {
			Select::having_flag = true;
		}
		if (stat->select->subquery->order_flag == true) {
			Select::order_flag = true;
		}
		Select::dele = false;
	}
	else
	{
		//error
	}
}
int Select::judge_int;
float Select::judge_float;
char* Select::judge_string;
Drop::Drop(std::shared_ptr<StatementAST> stat)
{
	Drop::stat = stat;
	if (!stat->drop->droptb) {
		Drop::drop_kind = 1;
		Drop::table_list = stat->drop->droptb->table_list;
	}
	else if (!stat->drop->dropindex) {
		Drop::drop_kind = 2;
		Drop::index_list = stat->drop->dropindex->index_list;
	}
	else
	{
		//error
	}
	
}

void Drop::drop_cata(string tbname)
{
	unsigned int tab_id;

	vector<Column> cols = vector<Column>();

	if (catalog::catamap.count(tbname) > 0) {
		tab_id = catalog::catamap[tbname];
	}

	string file = catalog::cata_path + std::to_string(tab_id) + ".log";
	remove(strdup(file.c_str()));

}

Result Drop::drop()
{
	if (Drop::drop_kind == 1) {
		std::vector<std::shared_ptr<IdAST>> table_list = Drop::stat->drop->droptb->table_list;
		for (std::shared_ptr<IdAST> tab : table_list) {

			string tbname = *tab->id;
			vector<Column> cols = Select::get_cols(*tab->id);
			Table* t = new Table(catalog::catamap[tbname], strdup(tbname.c_str()), cols);
			t->deleteRecord(string(strdup(tbname.c_str())), al_true);

		}
	}
	else {

	}
	return Result(1);
}

Insert::Insert(std::shared_ptr<StatementAST> stat)
{
	Insert::stat = stat;
	Insert::table_name = stat->insert->table_name;
	Insert::col_names = stat->insert->col_names;
	Insert::value_list = stat->insert->value_list;
}

Result Insert::insert()
{
	const char* tname =( *table_name->id).c_str();
	auto table = new Table(
		catalog::catamap[*table_name->id], 
		tname,
		Select::get_cols(*table_name->id));
	vector<char*> records;
	for (std::shared_ptr<ExprAST> val : Insert::value_list) {
		char *p = const_cast<char*>((*val->lhs->bp->p->bitexpr->bitexp->bitex->SE->id->id).c_str());
		records.push_back(p);
	}
	table->insertRecord(records);

	return Result(1);
}

Dele::Dele(std::shared_ptr<StatementAST> stat)
{
	Dele::stat = stat;
}

Result Dele::dele()
{
	Select select(Dele::stat);
	select.dele = true;
	return select.select();
}

Setvar::Setvar(std::shared_ptr<StatementAST> stat)
{
	Setvar::id = stat->setvar->id;
	Setvar::expr = stat->setvar->expr;
}

bool gteq_int(char* s)
{
	int v = ((int *)s)[0];
	return Select::gteq_int(s) ;
}
bool gteq_float(char* s)
{
	float v = ((float *)s)[0];
	return v >= Select::judge_float;
}
bool gt_int(char* s)
{
	int v = ((int *)s)[0];
	return v > Select::judge_int;
}
bool gt_float(char* s)
{
	float v = ((float *)s)[0];
	return Select::gt_int(s);
}
bool lteq_int(char* s)
{
	int v = ((int *)s)[0];
	return Select::lteq_int(s);
}
bool lteq_float(char* s)
{
	float v = ((float *)s)[0];
	return v <= Select::judge_float;
}
bool lt_int(char* s)
{
	int v = ((int *)s)[0];
	return Select::lt_int(s);
}
bool lt_float(char* s)
{
	float v = ((float *)s)[0];
	return v < Select::judge_float;
}
bool eq_int(char* s)
{
	int v = ((int *)s)[0];
	return Select::eq_int(s);
}
bool eq_float(char* s)
{
	float v = ((float *)s)[0];
	return v == Select::judge_float;
}
bool not_eq_int(char* s)
{
	int v = ((int *)s)[0];
	return Select::not_eq_int(s);
}
bool not_eq_float(char* s)
{
	float v = ((float *)s)[0];
	return v != Select::judge_float;
}
bool eq_string(char* v)
{
	return strcmp(v, Select::judge_string) == 0;
}
bool not_eq_string(char* v)
{
	return !strcmp(v, Select::judge_string) == 0;
}
bool al_true(char* v)
{
	return true;
}


