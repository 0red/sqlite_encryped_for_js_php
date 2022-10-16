<?php
error_reporting(E_ALL);
# SQLITE_ENABLE_CSV;SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION;SQLITE_ENABLE_DBSTAT_VTAB;SQLITE_ENABLE_JSON1;SQLITE_ENABLE_FTS5;SQLITE_ENABLE_RTREE=1;SQLITE_ENABLE_SESSION;SQLITE_ENABLE_PREUPDATE_HOOK;WIN32;SQLITE_ENABLE_RTREE=1;SQLITE_ENABLE_COLUMN_METADATA;SQLITE_HAS_CODEC;SQLITE3ENCRYPT_EXPORTS;SQLITE_ENABLE_FTS4;SQLITE_ENABLE_FTS3;SQLITE_ENABLE_FTS3_PARENTHESIS;SQLITE_CORE;SQLITE_ALLOW_URI_AUTHORITY;SQLITE_SOUNDEX;CODEC_TYPE=CODEC_TYPE_AES256;NDEBUG;_WINDOWS;_USRDLL;SQLITE3_EXPORTS;%(PreprocessorDefinitions)

# https://github.com/shenghe/FreeSQLiteEncryption or https://github.com/rindeal/SQLite3-Encryption





class jr_sqlite{
  public $_pass='';
  public $_file='';
  public $_debug=0;
  public $_rowid=false;
  public $_count=false;
  private $wynik=false;
  private $error=false;
  private $out=false;
  private $status=false;
  public static $sqlite_wrapper="jrsqlite3.exe";
  public $ajax_cmd="qjr";
  public $ajax_enable=array(''
      ,'POST'
      ,'GET'
  );
  public $last_exec='';
  
  

  public static $base64_long_table="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";

  public static function jrsqlite_exec($query,$pass='',$file='',$newpass='',$debug=0) {
      
      $tmp_file=self::$sqlite_wrapper.".tmp".getmypid().".txt";

      $sqlite=self::$sqlite_wrapper;
      $cmd='';
      if ($pass) $cmd.=" -p".escapeshellarg($pass);
      if ($newpass) $cmd.=" -n".escapeshellarg($newpass);
      if ($file) $cmd.=" -f".escapeshellarg($file);
      if ($debug) $cmd.=" -d";
      if ($query) {
          //$cmd.=" -q".escapeshellarg($query);
          $cmd.=" -g".escapeshellarg($tmp_file);
          file_put_contents($tmp_file,$query);
      }
      //print "\n$sqlite$cmd\n";//die();
      
      
//      self::json(array("cmd"=>"$sqlite$cmd","stdin"=>$query));
      ob_start();
      $proc=passthru("$sqlite$cmd",$return_value );
      @unlink($tmp_file);
      $wy=ob_get_clean();$err="";
      return array('ret'=>$return_value ? $return_value :"0",'out'=>$wy,'err'=>trim($err),'query'=>$query,'sh'=>"$sqlite$cmd");
  }


  public static function base64_long_decode($str) {
      $w=0;
      for ($i=0;$i<strlen($str);$i++) {
        $w=($w<<6) + strpos(self::$base64_long_table,$str[$i]);
      }
      return $w;
  }

  public static function base64_long_encode($long) {
      $w='';
      do {
        $w.=self::$base64_long_table[$long % 64];
        $long= $long>>6;
      } while ($long);
      $w="=".strrev($w);
      return $w;
  }

  function query($query="select * from Cars;",$_new_pass="") { //_new_pass='null' do delete password
      $this->_rowid=false;$this->_count=false;
      $w=$this->jrsqlite_exec($query,$this->_pass,$this->_file,$_new_pass,$this->_debug);
      $this->status=array();
      //print_r($w);
      $this->last_exec=$w['sh'];$this->out=$w['out'];
      if (!$w['ret']) {
        $wy=explode("\n",$w['out']);$res=array();$state=0;
        foreach ($wy as $k=>$v) {
          $v=trim($v); if (!$v) {unset($wy[$k]);continue;}
   //       print(__LINE__."$state\t$v\n");
          if ($v=='---jr-start---') {$state=1;continue;}
          if ($v=='---jr-end---')   {$state=3;continue;}
          
          switch ($state) {
            case 0:
              unset($wy[$k]);
              if ($v=='---jr-start---') $state=1;
              break;
            case 1:
              unset($wy[$k]);
              $res=array(explode("|",$v));
              $state=2;
              break;  
            case 2:
              unset($wy[$k]);
              if ($v=='---jr-end---') {
                $state=3;
              } else {
                $tmp=explode("|",$v);
                foreach ($tmp as $k1=>$v1) {
                  if ($v1[0]=="`") {
                    $tmp[$k1]=substr($v1,1);
                  } elseif ($v1=="~") {
                    $tmp[$k1]=null;
                  } elseif ($v1[0]=="=") {
                    //$tmp[$k1]=base64_long_decode(substr($v1,1))." ".$v1." ".base64_long_encode(base64_long_decode(substr($v1,1)));
                    $tmp[$k1]=$this->base64_long_decode(substr($v1,1));
                  } else {
                    $tmp[$k1]=base64_decode($v1);
                  }
                }
                array_push($res,$tmp);
              }
              break;    
            case 3:
              array_push($this->status,$v);
              if ($v=='SQL OK') { ;}
              if (preg_match('/rowid=(\\d+)/', $v, $regs)) {
                  $this->_rowid=$regs[1];
              }
              if (preg_match('/count=(\\d+)/', $v, $regs)) {
                  $this->_count=$regs[1];
              }
              break;           
          }
        }
        $this->wynik=$res;$this->error=$w['err'];          
        //print_r($res);

      } else {
        $this->error=$w['err'];$this->wynik=false;
      }
    }
    
    
    function __construct($pass="",$file="",$debug=1) {
      $storagelocation = (function_exists('exo_getglobalvariable')) ? exo_getglobalvariable('HEPubStorageLocation', '') :"";
      $storagelocation = "C:\\Users\\jrusin\\AppData\\Local\\ExeOutput\\UserApplication\\{3AE5192C-F8CC-477F-B0E0-73D6D84D5BF2}\\";
      $this->_pass=$pass;
      $this->_file=$storagelocation ? $storagelocation.'pmo.db':$file;
      $this->_debug=$debug;
      if ($storagelocation ) self::$sqlite_wrapper=$storagelocation.self::$sqlite_wrapper ;
    }
    
    function get($query='') {
      if ($query) $this->query($query);
      if ($this->error || $this->wynik===false) return false; // brak wyników
   
      $wy=array();
      foreach ($this->wynik as $k=>$v) {
        if (!$k) { $head=$v;continue;};
        $tmp=array();
        foreach ($v as $k1=>$v1) $tmp[$head[$k1]]=$v1;
        array_push($wy,$tmp);
      }
      return $wy;
    }
    
    function get_array($query='') {
      if ($query) $this->query($query);
      if ($this->error || $this->wynik===false) return false; // brak wyników
      return $this->wynik;
    }
    
    function get_error($query='') {
      if ($query) $this->query($query);
      return $this->error;
    }
    
    function ajax($query='') {
     // print "1";
      if (!$query) $query=isset($_POST[$this->ajax_cmd]) ? $_POST[$this->ajax_cmd] : $_GET[$this->ajax_cmd];
     // print "2";
      $w=$this->query($query);
     // print "3";
      if ($w===false) {
      //  print "4";
        $wy=array("status"=>"ERROR","error"=>$this->get_error());
      } else {
      //  print "5";
        $wy=array("status"=>"OK","result"=>$this->get()); 
      }
      //  print "6";
       $wy['sh']=$this->last_exec;$wy['error']=$this->error;$wy['out']=$this->out;$wy['status']=$this->status;
       
     // print "7";
     //   print_r($wy);
       self::json($wy);
    }
    
    public static function json($a) {
    if (!(isset($_SERVER) && isset($_SERVER['argv']))) 
          header('Content-Type: application/json');
      print json_encode($a);
      die();
    }
    
    public  function change_pass($new_pass='null') {
      $w=$this->jrsqlite_exec("SELECT 1;",$this->_pass,$this->_file,$_new_pass,$this->_debug);
      $this->_pass=$new_pass;
    }
    
    
    public function hash_create($new_pass='') {
      $query='create table if not exists jr_hash (k varchar PRIMARY KEY ASC,v varchar,d TEXT DEFAULT CURRENT_TIMESTAMP';
      $w=$this->jrsqlite_exec($query,$this->_pass,$this->_file,$_new_pass,$this->_debug);
    }
     
    public static function sq_str($str) {
      return join("''",explode("'",$str));
    }
      
    public function hash_set ($k,$v) {
    
      $query="INSERT OR REPLACE INTO jr_hash ('".self::sq_str($k)."','".self::sq_str($v)."',CURRENT_TIMESTAMP);";
      $w=$this->jrsqlite_exec($query,$this->_pass,$this->_file,$_new_pass,$this->_debug);
    }
    
}


//print_r($_SERVER);print_r($_POST);die();

$w=new jr_sqlite('pass','test.db');
if (isset($_SERVER) && isset($_SERVER['argv']) && $_SERVER['argv']) {
  $_POST[$w->ajax_cmd]=$_SERVER['argv'][1]; 
}


if (
  (in_array('POST',$w->ajax_enable) && isset($_POST) && $w->ajax_cmd && isset($_POST[$w->ajax_cmd])) 
  ||
  (in_array('GET',$w->ajax_enable) && isset($_GET) && $w->ajax_cmd && isset($_GET[$w->ajax_cmd])) 
  )   $w->ajax();

//var_dump($w); 
$w1=$w->ajax("select * from t1;");
print_r($w1);
var_dump($w); 
?>
