async function sql(q,bulk_insert=false) {
  if (bulk_insert) q=[
      "PRAGMA synchronous = OFF;",
      "PRAGMA locking_mode = EXCLUSIVE;",
      "PRAGMA journal_mode = OFF;",
      "PRAGMA temp_store = MEMORY;",
      "PRAGMA cache_size = 500000;",
      "PRAGMA count_changes = OFF;",
      "BEGIN TRANSACTION;",
      q,";COMMIT;"].join("\n");
  let x=await ajax_post("jr_sqlite_secure.php",{qjr:q},'json');;
  if (x.out) x.out=x.out.split("\n");
  console.log([q,x]);
  return x;
}

async function sql_key_set(key,v,create_table=false) {
  if (!isString(key)) return false;
  if (create_table) {
    let ba1='CREATE TABLE IF NOT EXISTS db (k varchar PRIMARY KEY, typ int, v varchar NOT NULL, ts varchar DEFAULT CURRENT_DATE);';
    await sql(ba1);
  }
  let ba,x;
  if ((isString(v) && v.length<120) || isNumber(v)) {
    x=await sql("INSERT OR REPLACE INTO db VALUES('"+key+"',1,'"+(v+"").split("'").join("''")+"',CURRENT_DATE);");
  } else {
    ba=btoa(str_compress(JSON.stringify(v)));
    x=await sql("INSERT OR REPLACE INTO db VALUES('"+key+"',2,'"+ba+"',CURRENT_DATE);");
  }

  return x;
}

async function sql_key_get(key,full=0) {
  if (!isString(key)) return false;
  let x=await sql("SELECT * FROM db where k='"+key+"';");
  if (!x || x.result===false || !x.result[0]) return false;
  let r=x.result[0];
  if (r.typ==2) {
    r=JSON.parse(str_uncompress(atob(r.v)));
  } else {
    if (isNumber(r)) r=r*1;
  }
  return full ? [r,x.result[0].ts,x.result[0].typ] : r;
}


function sql_str(s) {
  return (s+"").split("'").join("''");
}

function str_compress(str,double_chr=false) {
  // https://github.com/nodeca/pako
  let x=(pako && pako.deflate) ?pako.deflate(JSON.stringify(str),{to:'string'}) : str;
  if (double_chr!==false) x.split(double_chr).join(double_chr+double_chr);
  return x;
}

function str_uncompress(binaryString,double_chr=false) {
    if (double_chr!==false) binaryString.split(double_chr+double_chr).join(double_chr);
    return (pako && pako.inflate) ? JSON.parse(pako.inflate(binaryString, { to: 'string' })) : binaryString;
}

const isString = a => (!!a) && (a.constructor === String);
const isNumber = a => (!!a) && (a.constructor === Number);
