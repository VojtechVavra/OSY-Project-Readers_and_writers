# OSY-Project-Readers_and_writers
## Semestrální projekt do předmětu Operačních systémů(OSY) - Čtenáři a spisovatelé  
*14. 5. 2020*  

**Zadání projektu:**  
Projekt bude napsaný v programovacím jazyce C/C++ pod OS Unix.  
Implementace IPC problému: **čtenáři a spisovatelé**.  
  
Navrhněte a realizujte program klient/server přes síťové rozhraní socket, kdy server povolí připojení více klientům a bude demonstrovat některý z klasických IPC problémů s využitím synchronizačních nástrojů OS. IPC problémy jsou v literatuře dobře popsány, včetně jejich řešení. Hlavní je tedy jejich vhodná implementace, ne hledání jiných problémů, nebo vlastních řešení!  
  
U všech IPC problémů zadává klient do algoritmu své požadavky, proto i zde budou požadavky vznikat u klienta a server bude zpět zasílat klientovi informace o průběhu jejich řešení. Předávání informací navrhněte textovou formou - jako slovně vyjádřené příkazy s parametry (pokud to bude zapotřebí) - podle navrženého protokolu. Pro lepší představu, jaké zprávy si mezi sebou může klient a server zasílat, jsou připraveny zjednodušené sekvenční diagramy - holič, filozofové, **čtenáři a spisovatelé**. POZOR! Tyto diagramy nepopisují řešení IPC problémů! Řešení je popsáno v knize.  
  
Projekt serveru se bude skládat ze dvou částí, kdy IPC problém bude zpracován pomocí vláken i pomocí procesů. Klient musí být pro obě implementace totožný.  
  
Navržený protokol pro komunikaci mezi klientem a serverem. Délka každé zprávy musí být maximálně 256 znaků včetně posledního znaku \n. Zasílané zprávy budou v textové formě v následujícím formátu:  

    ?[NN]:[text]\n  
    
Kde význam jednotlivých znaků je následující:

    ? - typ zprávy povinný. C - command, A - Answer, W - warning, E - Error, I - Information.
    NN - dvě číslice kódu zprávy, volitelné.
    : - povinný oddělovač.
    text - libovolný volitelný ASCII text s informacemi.
    \n - povinný oddělovač nového řádku označovaný jako LF - ASCII(10).  
      
![cten-spis](/cten-spis.jpg)

*zdroj: http://poli.cs.vsb.cz/edu/osy/poz-denni.html*  
