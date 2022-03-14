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
 - **Number** - čísla - `1`,`25`,`3.14`,`-21.8`,`+10.25e6`,`4.8e-8` - interně se rozlišují celá čísla a čísla s plovoucí čárkou. Pokud jde matematická operace vyřešit v rámci celých čísel, ukládá se číslo jako celé, jinak je převedeno na číslo s plovoucí čárkou. Konstanta, která má být automaticky převedena na číslo s plovoucí čárkou musí mít uvedenou desetinnou tečku a aspoň jedno desetinné místo: `214.0` - je uloženo jako číslo s pohyblivou desetinnou čárkou. Pří zápisu čísel se ignorují bílé znaky (kromě znaku nové řádky), tedy lze mezerou oddělovat tisíce: `100 000`, `0.254 787`. 
 - **String** - `"Ahoj"`, `"Citace \"Ahoj\""`, `"Znak \\ zpětné lomítko"`, `"Nová\nřádka"`. Při zápisu řetězců platí stejná pravidla jako pro řetězce ve formátu JSON
 - **Boolean** - konstanta **true** nebo **false**
 - **Null** - hodnota **null** je považována za hodnotu. Proměnná nastavená na hodnotu null je považována za proměnnou mající hodnotu. Pokud proměnná není nastavena, pak je výsledkem **undefined**
 - **Undefined** - hodnota **undefined** - objevuje se zejména tam, kde výsledkem operace není žádná hodnota.
 - **Array** - `[1,2,3,4,5...]` -Pole je indexováno od nuly a může obsahovat libovolný mix typů. Pole je immutabilní, nelze změnit hodnotu uvnitř pole, změna jakékoliv hodnoty znamená vytvoření kopie pole
 - **Object** - `{"foo":10,"bar":true}` -  Objekty mají podobnou strukturu jako objekty JSONu. Objekty jsou immutabilní, objekt lze změnit jedině tak, že vznikne jeho kopie (která obsahuje změnu)
 - **seznam hodnot** - `(1,5,"test",true)` - Seznam hodnot se používá k předávání parametrů do funkcí, nebo vracení výsledku z funkce. Seznam hodnot nelze uložit do proměnné, při uložení se převede na pole. Pomocí operátor `...` lze převádět pole na seznam hodnot a naopak - viz popis param-packu
 - **rozsah** - Hodnota typu **rozsah** umožňuje definovat rozsah celočíselných hodnot, které se následně prezentuje jako pole po sobě jdoucích hodnot: `5..12`=`[5,6,7,8,9,10,11,12]`
 - **Block** - Část kódu, kterou lze samostatne spustit.
 - **Function** - Představuje hodnotu, kterou lze použít jako funkci - zavolat. Představuje funkce, metody, i objekty, které lze vyvolat jako funkci.

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

### Přiřazení výchozí hodnoty

```
A?=10
```
Přířadí hodnotu, pokud proměnná není definovaná, jinak ponechá původní

### Zakrytí proměnné 

Pokud chceme zakrýt existující proměnnou z předchozího scope

```
A=undefined
```

Tato proměnná už nesmí být měněna ale ve vnořeném scope nebude viditelná

### Zkopírování proměnné do aktuálního scope

```
use A
```



## Operátory a priorita

<table>
<tr><th> Priorita </th><th> Operatory </th><th> Význam </th><tr>
<tr><td>0</td><td> unární +,- </td><td>Unární plus a mínus před číslem</td></tr>
<tr><td>0</td><td> !,not </td><td>Logická negace</td></tr>
<tr><td>0</td><td> ? </td><td>Operátor ? se píše před proměnnou ?A a testuje, zda je definovaná</td></tr>
<tr><td>1</td><td> . </td><td>Dereference pole objektu</td></tr>
<tr><td>1</td><td> [x] </td><td>Dereference</td></tr>
<tr><td>1</td><td> -> </td><td>Přetypování a volání metody x->Objekt.metoda(...) </td></tr>
<tr><td>1</td><td> => </td><td>Deklarace funkce (x)=>{...} </td></tr>
<tr><td>2</td><td> ^ </td><td>Mocnina (X^2) </td></tr>
<tr><td>2</td><td> *,/,% </td><td>Násobení, dělení, modulo </td></tr>
<tr><td>3</td><td> +,- </td><td>Sčítání a odčítání </td></tr>
<tr><td>4</td><td> .. </td><td>Definice rozsahu (A..B) </td></tr>
<tr><td>5</td><td> in, <custom> </td><td>Test, zda proměnná je součástí objektu (A in B), uživatelem definované operátory </td></tr>
<tr><td>6</td><td> ==,>=,<=,<>,!=,>,<,= </td><td>Porovnávání</td></tr>
<tr><td>7</td><td> and </td><td>Logický součin</td></tr>
<tr><td>8</td><td> ?? </td><td>A??B - A pokud je definovane, jinak B</td></tr>
<tr><td>8</td><td> or </td><td>Logický součet</td></tr>
<tr><td>9</td><td> ?:, </td><td>Ternární operátor A?B:C</td></tr>
</table>


### Závorky
- **()** - asociativita, deklarace seznamu hodnot (a,b,c), parametry funkce
- **[]** - pole [1,2,3,4], index a[x]
- **{}** - deklarace bloku {...code...}


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

Smyčka for prochází kontejner (pole, objekt) a provádí kód pro každý prvek. Příkaz napodobuje provádění cyklu rekurzí, přestože rekurzivně se fyzicky neprovádí. Pro každý cyklus jsou k dispozici proměnné deklarované v předchozím cyklu. V tomto cyklu lze proměnné deklarovat znova a využít k proměnné z předchozího cyklu. Tím dochází "virtuálně" k přepsání proměnné. Před prvním cyklu jsou proměnné nastavené podle `<init>` části

```
for (<iterator>: <konteiner>, <init>)(
	<blok>
}
```

* **<iterator>** - proměná která funguje jako iterátor
* **<kontejner>** - kontejner, který se iteruje. Může být pole, nebo také rozsah `A..B`
* **<init>** - nepovinná část, čárkou oddělený seznam proměnných, které se inicializují před prvním cyklem. Povolen je jediný zápis a to `X=<vyraz>`

Příkaz se chová jako funkce, která vrací objekt, který obsahuje proměnné z posledního průchodu.

```
faktorial=(for (i:1..N, f=1) {
   f=f*i
}).f
```


### Smyčka while

```
while (<condition>) {
	<blok>
}
```

Smyčka se provádí tak dlouho, dokud je `<condition>` splněno. Jakmile není `<condition>` splněno, smyčka se ukončí. Pokud na začátku cyklu není `<condition>` splněno, neprovede se žádný cyklus.

Výraz `<condition>` se může odkazovat na proměnné přiřazené nebo změněné ve scope uvnitř bloku. 

```
N=10
F=1
faktorial=(while(N>1) {
   F=F*N
   N=N-1
}).F
#Po skončení cyklu budou proměnné obsahovat toto
#N=10, F=1, faktorial=N!
#Pozor že proměnné F a N se mění jen uvnitř bloku
```

Stejně jako u smyčky `for`, cyklus while napodobuje rekurzivní vykonávání tím, že v každém novém průchodu jsou k dispozici proměnné z předchozího průchodu a lze je tedy deklarovat znovu a tím je modifikovat. Příkaz vrací stav proměnných po vykonání posledního průchodu jako objekt. Pokud cyklus neproběhl ani jednou, výsledkem je `null`

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
foo.bar
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

#### closure

Funkci lze přetvořit v objekt, pak je vznikne closure

```
fn=object (x)=>{
	A+x
} {
	A=10	
}
```

Proměnná `fn` je v tomto případě objekt, který je zároveň funkcí, tedy může být zavolán jako funkce. Uvnitř bloku funkce má k dispozici sebe sama jako scope a může tedy používat proměnné toho scope


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


### Třídy

Třídy jsou objekty. Jsou standardně definované pro všechny typy. 

- funkce `typeof(x)` vrac jméno třídy pro daný type
- Třída zpravidla přináší metody pro práci s typem. Tyto metody jsou dostupné i přes tečkový operátor i přesto, že na levé straně není objekt

```
A=[1,2,3]
A.size()    # volá Array.size() nad proměnnou A

#Result: 3
```

#### Předefinování metod v třídách. 

Třídy se oslovují pomocí jména, proto je snadné předefinovat chování metod v třídách. Stačí vytvořit novou revizi třídy

```
Array=object Array {
		neco=()=>{
			#kod
		}
}
```

Nová definice je platná v tomto a ve všech vnořených scope


#### Přetypování

Při běžném zavolání metody se jako `this` předává objekt, jehož je metoda členem. Toto výchozí chování lze změnit použitím přetypování. Přetypování znamená zavolání metody z jiné třidy nad zadanou proměnnou bez ohledu na to, jakého je typu. K přetypování se používá operátor `->`

```
hodnota->Objekt.metoda(paramety)
```

```
X=object {
	A=1
	B=true
}
X->Array.size()

#Result: 2
```

Výše uvedený příklad funguje protože objekty lze indexovat jako pole, a proto existuje možnost přetypovat objekt na pole a zavolat metodu pole

Přetypování lze použít k zavolání funkce jakoby šlo o metodu a předat ji this. Lze tedy vytvářet metody, které nejsou vázané na objekt

```
fn=()=>{   #obyčejná funkce
  #kod
}

objekt->fn()  #zavolej funkci jako by to byla metoda, předej ji objekt
```

## Práce s poli

### Objekt jako pole

- Objekt lze použít jako pole. Jednotlivé položky jsou seřazený alfanumericky, tedy na indexu 0 bude pole které se podle jména řadí na první místo, atd...
- Objekt lze také iterovat pomocí `for`. Získáme hodnoty
- Při iteraci lze použít funkci `keyof(x]` k získání klíče
- Objekt má definované [], použijeme řetězcovou hodnotu pro přístup k hodnotám podle jména


### Velikost pole

```
pole.size()
```

### Indexace pole

Pole lze indexovat číselným indexem v rozsahu mezi 0 až (velikost-1). Indexace mimo rozsah vrací `undefined`

### Podrozsahy a mapování indexu

Použití rozsahu `X..Y` lze získat část pole

```
A=[10,20,30,40,50]
A[1..3]

#Result: [20,30,40]
```

Jako index lze použít jiné pole celočíselných hodnot, pak lze vytvořit nové pole s přemapovanými indexy

```
A=[10,20,30,40,50]
B=[0,2,4,2]
A[B]

#Result: [0,30,50,30]
```

Pozor, je rozdíl mezi A[1] a A[[1]]. V tom druhém případě je výsledkem jednoprvkové pole

### Spojování polí

Pole lze spojit pomocí operátoru `+`

```
A=[1,2,3]
B=["X","Y","Z"]
A+B
#Result: [1,2,3,"X","Y","Z"]
```
### Mapování matematickou operací

Pokud součástí matematické operace je pole a hodnota, pak se matematická operace aplikuje na všechny prvky pole za vzniku nového pole

```
A=[10,20,30]
(A/2, A+5, 20-A)

#Result: ([5,10,15], [15,25,35], [10,0,-10])
```
 
Matematické operace lze aplikovat i na rozsah (který se chová jako pole)

```
A=1..5
A*2
#Result: [2,4,6,8,10]
```
 
**Poznámka:** - operace má konstantní složitost, matematická operace je forma líného mapování, ke skutečnému výpočtu dochází až při přístupu k prvkům

### Modifikace pole

Vkládání do polí uvnitř lze řešit pomocí poskládání částí polí do nového pole

**Poznámka** - Všechny operace jsou řešeny jako forma mapování. Po aplikaci všech operací je vhodné zavolat metodu copy(), která "zapeče" veškeré mapovací operace do nového pole

```
A=[1,2,3,4,5]
B=["ahoj","nazdar"]
C=A[0..2]+B+A[3..4]

#Result: [1,2,3,"ahoj","nazdar",4,5]
```
"Zapečení" všech úprav

```
D=C.copy()
```

Pokud je například cílové pole skládáno v cyklu pomocí operace `+`, po mnoha cyklech může být indexace prvků vložených na začátku cyklu náročná na zpracování. Potom zavolání `copy()` vytvoří nové pole, ve kterém je přístup ke každému prvku opět konstantní

### Vkládání prvků na začátek a na konec

- **Array.push_back(x)** - vloží prvek na konec pole. Funkce vrací nové pole obsahující nový prvek.
- **Array.push_front(x)** - vloží prvek na začátek pole. Funkce vrací nové pole obsahující nový prvek.
- **Array.pop_back()** - Odebere poslední prvek z pole. Funkce vrací nové pole bez posledního prvku
- **Array.pop_front()** - Odebere první prvek z pole. Funkce vrací nové pole bez prvního prvku
- **Array.back()** - Vrací poslední prvek
- **Array.front()** - Vrací první prvek

Vkládání prvků na začátek a na konec je zvlášt optimalizováno, aby nedocházelo k zbytečným kopiim ani zřetězování operací nad polem. Proto operace vložení prvku je většinou rychlá bez ohledu na velikost pole (ale nemá konstantní složitost, protože v krajních případech se může stát, že dojde k vytvoření sloučené kopie pole, pokud je až příliž fragmentované v paměti)

Pokud vkládáte fragmenty polí, je vhodné využít vlastnost funkcí **push_back** a **push_front** v možnosti vložit na konec víc současně tím, že jsou všechny nové prvky zadány jako parametry (dva parametry vloží dva prvky). Pomocí operátoru `...` lze vložit na konec jiné pole

```
A=[1,2,3]
B=[4,5,6]
A.push_back(B...)

#Result: [1,2,3,4,5,6]
```

### Další operace

* **reverse()** - převrátí pořadí pole
* **indexOf** - najde prvek a vrací jeho index, -1 pokud nebyl nalezen
* **find(fn)** - volá funkci na každý prvek dokud funkce vrací `false`. Pokud vrátí `true`, procházení zastaví a vrátí nalezený prvek. Pokud není nalezený žádný, vrací `null`
* **findIndex(fn)** - jako **find** ovšem vrací index
* **sort(fn(a,b))** - seřadí pole, dodaná funkce postupně obdrží dvojice prvků a musí vrátit nulu, pokud jsou prvky rovny, záporné číslo, pokud je a<b, kladné číslo, pokud b>a
* **map(fn)** - provede mapování pole na jiné pole(které je vráceno). Na každý prvek zavolá funkci, předá ji 1-3 parametry `(prvek, index, celé_pole)`. Očekává se, že funkce transformuje předaný prvek na jiný prvek, který je poté vložen do nového pole. Funkce také může vrátit prázdný seznam hodnot `()`, potom je prvek pouze přeskočen, může však vrátit víc hodnot `(x,y,..)` pak jsou vloženy všechny vrácené prvky.
* **copy()** - veškeré matematické mapování, spojování polí, ale i vkládání prvků na konec (včetně operace `map()` převede na nové pole "ploché" pole.

## Matematické funkce

Veškeré matematické funkce jsou v třídě `Math`. Například `Math.sin()`
* **Math.E** - 2.718281828459045
* **Math.LN10** - 2.302585092994046
* **Math.LN2** - 0.6931471805599453.
* **Math.LOG2E** - 1.4426950408889634
* **Math.LOG10E** - 0.4342944819032518
* **Math.PI** - 3.141592653589793
* **Math.SQRT1_2** - 0.7071067811865476
* **Math.SQRT2** - 1.4142135623730951
* **Math.INF** - Nekonečno
* **Math.EPSILON** - Nejmenší krok čísla
* **Math.abs(x)** 
* **Math.acos(x)**
* **Math.acosh(x)**
* **Math.asin(x)**
* **Math.asinh(x)**
* **Math.atan(x)**
* **Math.atanh(x)**
* **Math.atan2(y,x)**
* **Math.cbrt(x)**
* **Math.ceil(x)**
* **Math.cos(x)**
* **Math.cosh(x)**
* **Math.exp(x)**
* **Math.expm1(x)**
* **Math.floor(x)**
* **Math.hypot(x)**
* **Math.log(x)**
* **Math.log1p(x)**
* **Math.log10(x)**
* **Math.log2(x)**
* **Math.max(x...)**
* **Math.min(x...)**
* **Math.pow(x,y)**
* **Math.round(x)**
* **Math.sign(x)**
* **Math.sin(x)**
* **Math.sinh(x)**
* **Math.sqrt(x)**
* **Math.tan(x)**
* **Math.trunc(x)**
* **Math.isfinite(x)** - vrací **true** pokud je dodaný argument konečné číslo
* **Math.isNaN(x)**  - vrací **true** pokud je dodaný argument číslo (vč. nekonečno)
* **Math.erf(x)** - spočítá gaussovu chybovou funkcu
* **Math.erfc(x)** - spočítá doplňek gaussovy chybové funkci
* **Math.tgamma(x)** - spočítá úplnou gamma funkci
* **Math.lgamma(x)** - spočítá logaritmickou gamma funkci
* **Math.expint(x)** - spočítá hodnotu exponenciálního integrálu
* **Math.beta(x,y)** - spočítá beta
* **Math.gcd(x,y)** - spočítá společné dělitele
* **Math.lcm(x,y)**


### Numerická integrace

Od zadané funkce lze spočítat numerický integrál dané funkce. 

```
IFN=Math.integral(fn,d,h,a)
```

* **IFN** jméno funkce, která obrdží integral dané funkce **fn**
* **fn** funkce (s jedním parametrem), kterou potřebujeme numericky integrovat
* **d** dolní mez integrace
* **h** horní mez integrace
* **a** stupeň přesnosti, definuje počet vzorků a tedy i přesnost výpoštu integrace. Velikost integrační tabulky je **2^a**, počet načtených vzorků je **3x2^a**. Výchozí hodnota je 8, maximální hodnota je 17

Hodnotu integrálu funkce v bodě X lze spočítat pomocí `IFN(x)`


```
i_sin=Math.integral(x=>Math.sin(x), 0, 2*Math.PI) # příprava integral pro sin v rozsahu 0-2*pi
i_sin(3)-i_sin(2)  # výpočet integrálu funkce sin v rozsahu 2-3
```

**Poznámka** -  příprava integrálu je výpočetně náročná operace. Proto je vhodné připravit integral během překladu, pakliže funkce je známá dopředu. Překladač automaticky spočítá integral během překladu pokud se funkce odkazuje pouze na funkce ze standardního globálního scope, případně funkce definované v současném bloku před výpočtem integralu a platí pro ně stejná pravidla jako pro integrovanou funkci.l

