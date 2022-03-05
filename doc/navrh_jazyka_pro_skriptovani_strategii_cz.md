## Jazyk pro skriptování strategii

(návrh)

* **inspirace** - pinescript

### Základní syntaxe

Jazyk má řádkovou strukturu. Jeden řádek, jeden příkaz (oddělovač je \n)

### Komentáře

Znak '#' zahajuje komentář - je interpretován jako konec řádku. Může se tedy vyskytovat na konci příkazu

### Datové typy

Základní datové typy zahrnují 
* čísla (reálná), 
* řetězce (utf-8), 
* pravdivostní hodnoty **true**/**false**, 
* hodnotu **null**. 

Strukturované typy představují 
* **pole** (pole indexované pořadím s prvním prvkem na indexu 0) 
* **objekt** (associativní pole indexované řetězcem)

Zápis jednotlivých hodnot odpovídá zápisu do formátu JSON.


#### Přířazení

```
PROM=<výraz>
```

* Přiřazení není výpočtový operátor. 
* Nelze přiřazovat více proměnným. 
* Nelze tedy A=B=C=10. 
* Nelze přiřazovat uvnitř výrazu: (10+(A=12*X))
* Proměnnou lze v rámci aktuálního kontextu přiřadit jen jednou. Opakované přiřazení je chyba.

#### Příkaz

```
<výraz>
```

Příkaz je výraz, jehož výsledek se zahodí

### Bloky

* Blok je kód, se kterým se zachází jako s hodnotou. 
* Blok začíná na místě, kde se očekává hodnota znakem `{`. Je to zároveň poslední znak na řádce
* Jednotlivé příkazy bloku pkračují na samostatných rádkách
* Blok končí znakem '}' na samostatném řádku. Pokud je blok součástí většího výrazu, pak výraz pokračuje ihned za blokem, nesmí být přerušen novou řádkou

```
{
  A=10
  B=true
  C=20
  D=A*B
  D*D
}
```

* pokud se blok evaluuje, pak výsledek posledního příkazu v bloku je výsledek.

### Klíčová slova a struktura kódu

#### Větvení

* příkaz `if`

```
if <vyraz> {
   # blok se vykoná při splněné podmínce 
}
```

```
if <vyraz> {
   # blok se vykoná při splněné podmínce 
} else {
   # blok se vykoná při nesplněné podmínce
}
```

```
if <vyraz> {
   # blok se vykoná při splněné podmínce 
} else if <vyraz> {
   # blok se vykoná při splněné druhé podmínce
} else {
   # blok se vykoná při nesplněné podmínce
}
```
Poznámka, mezi `else` a `if` nesmí být nová řádka, 

Doporučuje se výraz uzavírat do závorky, 

Každá součást příkazu může být nahražena proměnnou

```
if A B else if C D else E  # správný zápis
```
* A - boolean
* B - blok
* C - boolean
* D - blok
* E - blok

** poznámka ke scope **

Blok je exekuován v rámci aktuálního scope. Proměnná která je založena uvnitř bloku je viditelná vně bloku.


### Příkaz exec a práce se scope

Příkazem exec lze spustit blok bez podmínek ale s novým scope.

Scope představuje context nově vytvořených proměnných. Ve většině případů jsou jednotlivá scope vrstvena na sebe. Dokud není proměnná přiřazena, lze přistupovat k proměnným se stejným jménem z předchozího scope.

Příkaz `exec` vytvoří nové scope a v něm spustí blok

```
A=10
exec {
   # zde je A=10
   A=20
   # zde je A=20
}
# zde je A=10
```

Příkaz `exec` může vracet hodnotu, tou je výsledek poslední operace

Protože blok lze přiřadit do proměnné, lze jako parametr funkce `exec` použít proměnnou


#### Funkce

* Funkce přestavuje speciální blok - funkci lze přiřadit do proměnné

```
FN = (a,b,c) => {
  #blok
}
```

* Funkci lze zavolat FN(p1,p2,p3). 
* Výsledkem funkce je výsledek výrazu posledního příkazu, pokud to není přiřazení

##### Funkce jako dva vnořené exec

Funkce by se dala představit jako 2x vnořený exec, zejména při vyvolání

FN(1,2,3)

```
exec {
   a=1
   b=2
   c=3
   exec {
      #blok
   }
}
```


#### Vytvoření objektu

Scope lze použít k vytvoření nového objektu pomocí příkazu object

```
obj1=object 
{
    a=10
    b=true
    c=null
}
```
Výsledkem je objekt `obj`, který má tři proměnné a,b,c

```
obj1.b # výsledek je true
```

Příkaz `object` lze použít pro změnu existujícího objektu

```
obj2=object obj1 {
   d=33
}
```

Proč nelze jednoduše napsat `obj1.d=33` - viz immutabilita

##### hodnota undefined

Přazení `undefined` do proměnné má význam jen v rámci bloku `object` a znamená, že ve výsledném objektu bude zvolená proměnná smazána
 
```
obj3=object obj2 {
	b=undefined
	c=undefined
	e="hello"
}
# v obj3 bude a=10, d=33, e=hello
```
 
#### Příkaz with

Příkaz with exekuuje blok v rámci objektu

```
Z=with obj2 {
   a+d
}

# Z = 43
```

Je to ekvivalentní jako napsat Z=obj2.a+obj2.d

#### Metody

Pokud je funkce umístěna do objektu a následně volána pomocí tečkového operátoru, je umístěna do scope
kde nadřazeným scope je objekt, přes který byla zavolána

```
obj3=object obj2 {
	method1=FN
	method2=(x)=>{
	   # blok
	}
}


obj3.method1(1,2,3)
obj3.method2(true)
```

Funkce je zavolána následovně

```
with obj3 {
  exec {
    a=1
    b=2
    c=3
    exec method
  }
}
```

### Příkaz this

Příkaz this vrací objekt představující nadřazený scope, pokud je funkce volaná v rámci objektu - vrací
onen objekt. Objekt nelze měnit (viz Immutabilita dat) ale lze jej použít ke konstrukci nového objektu

```
object this {
   A=20  # změní proměnnou A v objektu this 
} # nový objekt je vrácen jako výsledek 
#pokud je to na konci funkce, je nový objekt vrácen z funkce jako výsledek
```

This lze použít k přístupu k proměnným z nadřazeným scope, pokud byly přepsány lokální verzi

```
A=20   # lokální verze
this.A  # původní hodnota
```

**Poznámka** - `this` vždy vrací kopii nadřazeného scope.

```
me=this # proměnná me bude obsahovat stav nadřazeného scope v tento okamžik. 
        # Kdykoliv se tento scope změní, bude proměnná pořád obsahovat původní kopii
```

#### příkaz use

Příkaz `use` importuje proměnnou do aktuálního scope

```
use A B C...
```

Příkaz je eqvivalentní zápisu `A=A` který vizuálně nedává smysl. 

```
A=10
OBJ=object {
	use A
	B=20
}
# OBJ obsahuje A=10, B=20
```

#### lambda funkce

Obyčejná funkce nemá vlastní scope a při vyvolání má dostupný nadřazený scope, což umožňuje vytvářet lambda funkce pro okamžité použití. 

Metoda ovšem na vrch záobníku vkládá ještě objekt, jehož je členem.

Další možností jsou lambda funkce s předem definovaným scope, který se vkládá při vyvolání funkice. Taková funkce je speciálním objektem ale i nadále má schopnost být zavolána

```
LambdaFN = object (args)=>{
  #function body
} {
  use A B X
  Z=123
}

LambdaFN(100)  # zavolá funkci a ta "uvidí" proměnné A B X a Z s hodnotou 123

LambdaFN2 = object Fn2 {  #Fn2 je funkce
  use X
}

```

Pokud je taková funkce použita jako metoda, pak může vidět i proměnné vlastního objektu, ale proměnné ze svého scope uvidí přednostně, pokud dojde ke kolizi jmen

Vlastní scope je read only. Pokud je třeba vytvořit nové scope, použije se opět příkaz `objekt`

```
LambdaFN2 = object LambdaFN2 {
   Z=222
}
```

Uvnitř funkce se `this` odkazuje na vlastní objekt funkce (pozor v případě this u metod)


### Immutabilita dat

Veškeré složitější datové struktury jsou automaticky immutabilní, týká se zejména objektů a polí. Jakmile je objekt založen, nelze jej změnit. Lze ovšem provést modifikaci za vzniku nového objektu (viz příkaz `objekt`)


Toto pravidlo dělá práci s objekty neintuitivní, nicméně přináší výhody z lepší konzistenci dat, které se nemohou měnit pod rukama

Pokud jsou deklarované metody v rámci objektů, pak je třeba rozlišit na metody které mění stav a které
pouze počítají výsledek. Lze umožňit jednu nebo druhou operací.

Metody které mění stav totiž musí vracet změněný stav objektu

```
Point = (x,y) => {
   object {  #přímo použij x,y jako member proměnné
      use x    
      use y
      move_x = (d) => {  #metoda move_x
      	object this {   #vem this
      		x=x+d       #změn proměnnou x 
      	}               #vrať výsledný objekt
      }
      move_y = (d) => {   
      	object this {
      		y=y+d
      	}
      }
   }
}

pt=Point(10,20)
pt2=pt.move_x(4)
pt3=pt.move_y(2)
pt4=pt2.move_y(2)
```

Pokud má metoda měnit stav a zároveň vracet výsledek, pak je k dispozici několik možností

* uložit výsledek do nového stavu
* vrátit objekt a výsledek v poli



### Pole

Pole je proměnná, která může obsahovat víc než jednu hodnotu. Pole není typové, může obsahovat různé typy hodnot. Pole vznikne použitém operátoru čárka

```
A=1,2,10,"ahoj"
```

Čárka má nejnižší prioritu

```
B=1+5,"Ahoj"+"Nazdar",object {
  X=10
  Y=20
},(5,6,8)
```

Je vhodnější zabalit pole do závorek (...)

```
A=(1,2,10,"ahoj")
```

#### Sloučení dvou polí

Operátor + se použije na sloučení dvou polí

```
A=(1,2,3)
B=(10,20,30)
C=A+B # C=(1,2,3,10,20,30)

D=C+("ahoj","nazdar")
```






