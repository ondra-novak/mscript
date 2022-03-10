# MScript - popis


## Vlastnosti

 - fokus na matematické výpočty
 - strukturované typy jsou immutabilní
 - funkcionální styl programování
 - jeden příkaz na jednu řádku
 - proměnné ve scope
 - funkce jsou anonymní, lze je uložit do proměnné
 - objekty jsou anonymní
 - jeden příkaz na řádku - oddělovač je nová řádka
 - klasické struktury - if-else, smyčky for a while
 - deklarace a procházemí rozsahů, mapování a reduce
 - cokoliv vrací hodnotu
 - poslední výsledek je výsledek bloku
 
 
## Základní syntaxe

Znak # uvozuje komentář, který pokračuje až na konec řádky

Na každou řádku lze zadat buď výraz, nebo přiřazení hodnoty

**Výraz**

```
(A*B+10)/(5*sqrt(2))
```
**Přiřazení**

```
vysledek=(A*B+10)/(5*sqrt(2))
```

Lze přiřadit pouze do jedné proměnné (A=B=10 není správně)
V rámci scope/bloku lze přiřadit do proměnné pouze jednou

```
A=10
A=20 #chyba
```

Proměnné nemají typ, dokud nejsou přiřazeny. Nelze dopředu definovat typ proměnné. Jazyk nemá typovou kontrolu

## Datové typy

 - **čísla** - `1`,`25`,`3.14`,`-21.8`,`+10.25e6`,`4.8e-8` - interně se rozlišují celá čísla a čísla s plovoucí čárkou. Pokud jde matematická operace vyřešit v rámci celých čísel, ukládá se číslo jako celé, jinak je převedeno na číslo s plovoucí čárkou. Konstanta, která má být automaticky převedena na číslo s plovoucí čárkou musí mít uvedenou desetinnou tečku a aspoň jedno desetinné místo: `214.0` - je uloženo jako číslo s pohyblivou desetinnou čárkou. Pří zápisu čísel se ignorují bílé znaky (kromě znaku nové řádky), tedy lze mezerou oddělovat tisíce: `100 000`, `0.254 787`. 
 
 - **řetězce** - `"Ahoj"`, `"Citace \"Ahoj\""`, `"Znak \\ zpětné lomítko"`, `"Nová\nřádka"`. Při zápisu řetězců platí stejná pravidla jako pro řetězce ve formátu JSON
 - **boolean** - konstanta **true** nebo **false**
 - **hodnota null** - null je považována za hodnotu. Proměnná nastavená na hodnotu null je považována za proměnnou mající hodnotu. Pokud proměnná není nastavena, pak je výsledkem **undefined**
 - **hodnota undefined** - objevuje se zejména tam, kde výsledkem operace není žádná hodnota.
 - **pole** - `[1,2,3,4,5...]` -Pole je indexováno od nuly a může obsahovat libovolný mix typů. Pole je immutabilní, nelze změnit hodnotu uvnitř pole, změna jakékoliv hodnoty znamená vytvoření kopie pole
 - **objekt** - `{"foo":10,"bar":true}` -  Objekty mají podobnou strukturu jako objekty JSONu. Objekty jsou immutabilní, objekt lze změnit jedině tak, že vznikne jeho kopie (která obsahuje změnu)
 - **seznam hodnot** - `(1,5,"test",true)` - Seznam hodnot se používá k předávání parametrů do funkcí, nebo vracení výsledku z funkce. Seznam hodnot nelze uložit do proměnné, při uložení se převede na pole. Pomocí operátor `...` lze převádět pole na seznam hodnot a naopak - viz popis param-packu
 - **rozsah** - Hodnota typu **rozsah** umožňuje definovat rozsah celočíselných hodnot, které se následně prezentuje jako pole po sobě jdoucích hodnot: `5..12`=`[5,6,7,8,9,10,11,12]`
 - **blok** - Část kódu, kterou lze samostatne spustit.
 - **callable** - Představuje hodnotu, kterou lze použít jako funkci - zavolat. Představuje funkce, metody, i objekty, které lze vyvolat jako funkci.

## Scope a přiřazení do proměnných

 - každý kód se vykonává v bloku, který je vymezen znaky `{` a `}`. Hlavní program nemá toto vymezení, tam je celým blokem považován celý textový soubor
 - blok má též definován scope, což je rozsah platnosti proměnných. Proměné deklarované ve scope jsou smazány, jakmile je scope opuštěno
 - jediný způsob, jak exportovat výsledek z bloku je prostřednictvím návratové hodnoty, třeba pomocí **seznamu hodnot**
 - funkce má také definované scope
 - scope se do sebe vnořují
 - do každé proměnné lze v rámci scope přiřadit **pouze jednou**
 - proměnné definované v předchozím scope jsou viditelné v nejvyšším scope. Nedají se ovšem měnit, jsou pouze pro čtení
 - Lze přepsat proměnnou z předchozího scope ovšem proměnná bude vytvořena v aktuálním scope

```
 A=10
 B=exec {
   A=A+5   #změn A na 10+5 = 15
   A+5     #vrat 15+5 = 20 -> uloz do B
 }
 # A=10, B=20
```


### Scope s kontextem

Scope může být inicializováno proměnnými z předchozího cyklu (u cyklů) nebo z objektu (`object`, `with`). Tyto proměnné se chovají stejně jako proměnné z předchozího scope (jako by bylo vloženo jedno extra scope mezi aktuální a předchozí). Nové proměnné jsou ukládány do nového scope. U cyklů nebo u stktury `objekt` pak dochází k vytvoření nového objektu který obsahuje původní proměnné pokud se nezměnily a nové změněné proměnné

```
obj1=object {
   A=1
   B=2
}
obj2=object obj1 {
   A=3
}
#obj1={"A":1,"B":2}, obj2={"A":3,"B":2}
```


## Přiřazení 

Přiřazení hodnoty do proměnné se zapisuje pomocí operátoru `=`

```
<proměnná>=<výraz>
```

Pokud výsledkem operace je **seznam hodnot**, pak lze jednotlivé hodnoty přiřadit naráz do několika proměnných

```
(A,B,C)=(1,2,3)

#A=1
#B=2
#C=3
```

Pomocí symbolu `-` lze konkrétní část výsledku ignorovat

```
(A,-,C)=(1,2,3)

#A=1
#C=3
```

Za poslední proměnnou mohou být uvedeny `...` (tři tečky), říká, že zbytek hodnot se má uložit jako pole do poslední proměnné

```
(A,B,C...)=(1,2,3,4)

#A=1
#B=2
#C=[3,4]
```

Je-li počet proměnných menší, než počet hodnot, jsou zbývající hodnoty ignorovány. Pokud je počet proměnných víc než počet hodnot, jsou zbývající proměnné nastaveny na hodnotu `null`

Pokud je seznam hodnot uložen do jedné proměnné, je seznam převeden na pole. Rozlišuje se, zda jsou použity závorky nebo nejsou

```
(A)=(1,2,3,4)
B=(1,2,3,4)

#A=1
#B=[1,2,3,4]
```


## Operátory

- `+`,`-`,`*`,`/` - matematické operátory, s obvyklou prioritou
- `^` - operátor mocniny, má nejvyšší prioritu
- `(`,`)` - závorky jak ve smyslu matematickém, tak ke konstrukcí seznamů hodnot
- `,` - symbol čárky uvnitř závorek (...,...) odděluje výsledky v rámci seznamu hodnot
- `{`,`}` - začátek a konec bloku
- `[`,`]` - deklarace pole, nebo indexu v poli či proměnné v objektu
- `=`,`>`,`<`,`!=`,`<=`,`>=`,`<>`,`==` - porovnávání. Operátory = a == oba mají význam porovnání v rámci výrazu. Nicméně operátor = může být zaměněn jako přiřazení u výrazu, která není přiřazením, pak použijte ==. Operátory != a <> mají stejný význam. Tyto operátory mají menší prioritu než matematické operátory
- `and`, `or`, `not` - operátory logických operací operací, mají menší prioritu než porovnávání. `and` má vyšší prioritu než `or`
- `..` - definice rozsahu A..B - má prioritu vyšší než porovnávání a nižší než matematické operace
- `?:` - klasický operátor IF-ELSE v rámci výrazu, nejnižší priorita
- `=>` - deklarace funkce
- `...` - práce s param packem

## Struktura programu

### Větvení If-else

```
if (vyraz) {
	akce_splneno
} else {
	akce_nesplneno
}
```

```
if (vyraz) akce_splneno 
else akce_nesplneno
```

```
if (vyraz) akce_splneno else akce_nesplneno
```

```
A=if (vyraz) vyraz_splneno else vyraz_nesplneno
```

Příkaz `if` funguje jako funkce, která může vracet hodnotu. Výsledkem je vysledek provedené větve

Použijte blok, pokud se operace nedají napsat do jednoho výrazu. Poslední výraz v bloku je návratová hodnota. Pokud není použit blok, lze zapsat pouze jeden výraz. Následující `else` může být uvedeno na samostané řádce.

**Pozor:** Výjimka- bloky **nevytváří scope** - proměnné deklarované v bloku stále náleží do scope celého nadřazeného bloku

#### if-else if-else


```
if (vyraz) {
	akce_splneno
} else if (vyraz2) {
	akce_splneno2
} else {
	akce_nesplneno
}
```


```
if (vyraz) akce_splneno
else if (vyraz2) akce_splneno2
else akce_nesplneno
```

```
if (vyraz) akce_splneno else if (vyraz2) akce_splneno2 else akce_nesplneno
```

**Pozor** - mezi `else` a `if` není povolen žádný další kód (výjma komentáře)

else větev není povinná. Pokud není splněna podmínka, výsledkem příkazu je `null`


### Větvení switch-case

```
switch (vyraz) {
    case c1: <vyraz>
    case c2,c3,c4: <vyraz>
    case c5: {
        <blok>
    }
    default: <vyraz>
}
```

Stejně jako `if` is příkaz `switch` vrací výsledek, který může být přiřazen. Výsledkem příkazu `switch` je výsledek vybrané větve. Není-li vybraná žádná větev, výsledkem je `null`

Hodnotami `c1...cx` musí být konstanty a musí být unikátní

Oproti C++ a javascriptu se **nepoužívá** `break`. Fallthrough není možný

**Pozor:** Výjimka - bloky **nevytváří scope** - proměnné deklarované v bloku stále náleží do scope celého nadřazeného bloku

**Poznámka:** Klíčová slova `case` a `default` nejsou vyhrazená slova. Je tedy možné přiřadit hodnotu do proměnných, které se stejně jmenují

```
default=1
switch (X) {
   default: default
}
```

### Smyčka for

Smyčka for prochází kontejner (pole, objekt) a provádí kód pro každý prvek. Příkaz funguje jako funkce, výsledkem je výsledek posledního výrazu bloku po provedení posledního cyklu

```
for (<iterator>: <konteiner>, <init>)(
	<blok>
}
```

* **<iterator>** - proměná která funguje jako iterátor
* **<kontejner>** - kontejner, který se iteruje. Může být pole, nebo také rozsah `A..B`
* **<init>** - nepovinná část, čárkou oddělený seznam proměnných, které se inicializují před prvním cyklem. Povolen je jediný zápis a to `X=<vyraz>`

```
faktorial=for (i:1..N, f=1) {
   f=f*i
}
```

Proměnné uvnitř bloku mohou být přepisovány pouze jednou, jejich hodnoty se převádí do dalšího cyklu. Po skončení cyklu jsou proměnné smazány a pouze výsledek poslední operace je vrácen jako výsledek celého cyklu.

### Smyčka while

```
while (<condition>) {
	<blok>
}
```

Smyčka se provádí tak dlouho, dokud je `<condition>` splněno. Jakmile není `<condition>` splněno, smyčka se ukončí. Pokud na začátku cyklu není `<condition>` splněno, neprovede se žádný cyklus a výsledkem operace je `null`

Výraz `<condition>` se může odkazovat na proměnné přiřazené nebo změněné ve scope uvnitř bloku. 

```
N=10
F=1
faktorial=while(N>1) {
   F=F*N
   N=N-1
   F
}
#Po skončení cyklu budou proměnné obsahovat toto
#N=10, F=1, faktorial=N!
#Pozor že proměnné F a N se mění jen uvnitř bloku
```

### Deklarace funkce a bloku

#### deklarace bloku 
```
myblok={
   print("hello")
}
```

Proměnná `myblok` obsahuje blok

#### příkaz exec

Blok lze spustit příkazem `exec`

```
exec myblok
```

Při exekuci bloku vzniká samostatný scope

Výhodou bloku je provedení nějaké operace, která vyžaduje deklaraci proměnných, kterými nechceme zaplevelovat aktuální scope

```
vysledek=exec {
   #nějaka složitá operace
}
```

#### deklarace funkce

```
(<argumenty>)=>{
   <tělo funkce>
}
```

Deklarace funkce se vyznačuje použitím binárního operátoru `=>`, přičemž na levé strany se uvádí proměnné pro argumenty, na pravé straně pak blok, který se exekuuje, pokud je funkce vyvolána

Argumenty obsahují jen výčet čárkou oddělených proměnných uzavřených do obyčejných závorek. Pokud funkce nemá žádné parametry, pak se uvedou jen prázdné závorky

Pokud má funkce jeden argument, pak závorky nejsou povinné

```
()=>{...}          #1
(a)=>{...}         #2
a=>{...}           #3
(a,b,c)=>{...}     #4
(a,b,c...)=>{...}  #5
```

 1. funkce bez parametrů
 2. funkce s jedním parametrem
 3. funkce s jedním parametrem
 4. funkce s třemi parametry
 5. funkce s variabilním počtem parametrů, přičemž první dva parametry se předají do prvních dvou proměnných, třetí proměnná obdrží pole se zbývajícími parametry

#### volání funkce

Funkci zpravidla uložíme do proměnné. Funkci pak zavoláme uvedením proměnné a závorek s parametry (závorky jsou povinné vždy)

```
fn=(a...)=>{...}

fn()               #1
fn(1)              #2
fn(A)              #3
fn(A,B)            #4
fn(X...)           #5
fn(X...,Y)         #6
fn(X...,Y...)      #7
```

 1. volání bez parametrů
 2. volání s jedním parametrem, konstantou
 3. volání s jedním parametrem
 4. volání s dvěmi parametry
 5. volaní s tím, že v X je pole, které se rozbalí na parametry (podle počtu hodnot v poli)
 6. jiný způsob expanze pole na parametry, nakonec se přidá parametr Y
 7. volání s možností expandovat víc polí do parametrů 


### Práce s objekty

#### Konstrukce objektu

Objekt vznikne sbalením aktuálního scope do objektu. To provádí příkaz `object`

```
obj=object {
   foo=1
   bar=false
   baz="yep"
}
```

Příkaz `object` ignoruje návratovou hodnotu bloku a vrací scope jako objekt


Pomocí `object` lze objekty i měnit. Změnou vznikne objekt nový (starý zůstává zachován). Následující kód demonstruje změnu pole `foo` a uložení celého objektu do nové proměnné


```
obj2=object obj {
    foo=2
}
```


Příkaz `object` zakládá scope s kontextem, kontextem v tomto případě je objekt, který je předmětem změny. Jeho proměnné jsou dostupné pouze pro čtení, ale mohou být použity k vytvoření nové revize objektu

```
obj3=object obj2 {
    foo=foo+1
}
```

#### Dereference

Operátor tečka `.` se použije jako dereference 

```
foo.bar``
```

Derefernci lze provést i pomocí indexu s použitím řetězcové hodnoty

```
foo["bar"]
```

**Poznámka** - volání metod lze provést pouze pomocí operátoru tečky (viz dále). 

#### Výpočty v rámci objektu - příkaz with

Příkaz pouze převede objekt do scope a exekuuje blok v rámci toho scope

```
obj=object {
	A=1
	B=2
}
res=with obj {
	A+B
}
#res=3
```


#### metody objektu

Funkce může být součastí objektu. Pokud je taková funkce zavolána přes tečkový operátor, je objekt namapován jako kontext nového scope

```
#017_methods_call.mscript

obj=object {
	A=42
	fn=(X)=>{
		A+X
	}
}
B=obj.fn(10)
#B=52
```

**Pozor** - pokud je funkce volána přes indexaci ["xxx"], pak to není považováno jako volání metody. Toto omezení lze obejít použitím `with`

```
B=obj["fn"](10) # chyba, A není definováno


B=with obj {
	this["fn"](10)
}
	
```

#### clousure

Funkci lze přetvořit v objekt, pak je vznikne closure

```
fn=object (x)=>{
	A+x
} {
	A=10	
}
```

Proměnná `fn` je v tomto případě objekt, který je zároveň callable, tedy může být zavolán jako funkce. Uvnitř bloku funkce má k dispozici sebe sama jako scope a může tedy používat proměnné toho scope


Pokud je funkce zároveň metoda, pak proměnné samotného objektu mají nižší prioritu před closure

```
obj=object {
     A=1
     B=2
     fn=object (x)=>{
        (A+x,B+x)
     } {
     	A=3     
     }
}

(X,Y)=obj.fn(3)
#X=6, protože A=3 má vyšší prioritu než A v objektu
#Y=5, protože B je deklarován v objektu
```
     
#### hodnota this

Hodnota `this` je proměnná, která ukazuje na aktuální objekt. Je potřeba k vytvoření nové revize objektu


```
object this {  # mění se aktuální objekt
 	foo=1
}
```
  
**Poznámka:** Identifikátor `this` není vyhrazené slovo. Mimo objekt lze použít jako proměnnou, které lze přiřadit hodnotu.


#### hodnota closure

Hodnota `closure` je proměnná, která je dostupna pouze v rámci funkce a obsahuje právě vyvolanou funkci, pokud je deklarovaná jako **closure**. Účelem je mít možnost modifikovat proměnné v closure a vrátit novou revizi funkce.

**Poznámka:** Identifikátor `closure` není vyhrazené slovo. Mimo funkci lze použít jako proměnnou, které lze přiřadit hodnotu.


```
object closure { # mění se aktuální closure
   foo=1
}
```

