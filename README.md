# sqlite_encryped_for_js_php

Please copy exe + dll i.e. into the dir where the server have the html/php files. (for exeoutput %LocalAppData%\ExeOutput\{App GUID})
For php usage please look into jr_sqlite_secure.php

I'm using it as the JS frontend to encrypted sqlite in EXEOUTPUT https://www.exeoutput.com/
For JS usage - the php (jr_sqlite_secure.php) as a wraper is requested (as well as proper access right)

when installed try in chrome console 
```javascript
{
  let q=[
                "DROP TABLE IF EXISTS Cars;",
                "CREATE TABLE Cars(Id INT, Name TEXT, Price INT);" ,
                "INSERT INTO Cars VALUES(67, 'A`udi', 52642);" ,
                "INSERT INTO Cars VALUES(2, 'M\"ercedes', 57127);" ,
                "INSERT INTO Cars VALUES(3, 'Skoda', 9000);" ,
                "INSERT INTO Cars VALUES(4, 'Volvo', 29000);" ,
                "INSERT INTO Cars VALUES(500, 'Bentley', 350000);" ,
                "INSERT INTO Cars VALUES(6000, 'Citroen', 21000);" ,
                "INSERT INTO Cars VALUES(7, 'Hummer', 41400);" ,
                "INSERT INTO Cars VALUES(8, 'Volkswagen', null);",
                "UPDATE Cars SET Name='VW' where Name='Volkswagen';"
         ];
  await sql(q);
         
  let result=await sql('select * from Cars');
  console.log(result.result)
}
```

based on https://github.com/utelle/wxsqlite3 via https://github.com/shenghe/FreeSQLiteEncryption or https://github.com/rindeal/SQLite3-Encryption 
