# OS-project-2022-aueb

Εργασία Λ.Σ. - Αναφορά

Για να αποθηκεύουμε όλα τα δεδομένα που χρειαζόμαστε για κάθε 
πελάτη-νήμα, φτιάξαμε ένα struct. Το τι προσδιορίζει η κάθε μεταβλητή 
του φαίνεται στα σχόλια του κώδικα.
Αρχικοποιήσαμε όλα τα δεδομένα μας και δημιουργήσαμε τα 
κατάλληλα mutex, cond. Το πρόγραμμά μας καλεί με τη 
σειρά τις μεθόδους:


1. int main(…)

2 παράμετροι (arg): 1 για τον αριθμό των πελατών και 1 για 
το seed. Κάνουμε Initialize ένα εικονικό πλάνο του θεάτρου 
μας, και τα 4 lock, 2 signal. Δημιουργούμε τα νήματα των 
πελατών σύμφωνα με τον αριθμό που μας έδωσε ο 
χρήστης. Βάζουμε ένα id για το καθένα από αυτά και τα 
περνάμε στη theater(). Κάνουμε sleep ένα random time (με 
βάση τα δεδομένα μας) αν το id δεν είναι 1 δηλαδή δεν είναι 
ο πρώτος πελάτης. Αφού τρέξουν όλα τα νήματα τα κάνουμε 
join πίσω στη main και τα καταστρέφουμε. Τυπώνουμε το 
εικονικό μας θέατρο στο τέλος.


2. void* theater(…)

Αυτή η μέθοδος εκτελείται για κάθε thread πελάτη. 
Παίρνουμε σαν παράμετρο τα thread id, κάνουμε memory
allocate για το struct μας, βάζουμε τοπική μεταβλητή το id
του thread και το αποθηκεύουμε στο αντίστοιχο πεδίο του 
struct μας. Κάνουμε Initialize τα service + waiting time στο 
struct, και μεταφερόμαστε στη μέθοδο για να κλείσουμε 
θέσεις στον πελάτη getSeats(). Εφόσον καταφέρει αυτός να 
κλείσει τις θέσεις, προχωράμε στην πληρωμή των θέσεων 
pay(). Σταματάμε το clock και αποθηκεύουμε τον χρόνο 
εξυπηρέτησης στο struct με τις πληροφορίες του πελάτη. 
Υπολογίζουμε και τους συνολικούς χρόνους εξυπηρέτησης 
και αναμονής με χρήση 2 αθροιστών: waitsum, tsum.
Ανάλογα με το status του πελάτη στο struct, τυπώνουμε 
ανάλογο μήνυμα:
- Αποτυχίας επειδή δεν υπάρχουν θέσεις (0)
- Αποτυχίας λόγω μη έγκυρης συναλλαγής (1) 
- Επιτυχίας, εμφανίζοντας τη ζώνη, σειρά και αριθμό 
θέσεων, καθώς και την συνολική τιμή για αυτές.
Εδώ τελειώνει το τρέχον thread του πελάτη.


3. int getSeats(…)

Αρχικά, κάνουμε wait αν οι operators είναι 0, μόλις μπει 
όμως ένα thread κάνουμε lock και μειώνουμε τους operators
για να δηλώσουμε ότι εξυπηρετούν ένα thread customer. 
Μειώνουμε το cashiers και κάνουμε unlock το mutex του 
(πήρε κάποιον πελάτη). Προσθέτουμε τον χρόνο αναμονής 
για τον πελάτη μέχρι τώρα και τον αποθηκεύουμε στο 
t_waiting.
Γίνεται η επιλογή θέσεων με τον εξής τρόπο: 
χρησιμοποιούμε τυχαία τιμή (με χρήση rand_r και τις 
τιμές/πιθανότητες που μας δόθηκαν) για την Ζώνη καθώς 
και για τον αριθμό θέσεων, με παρόμοιο τρόπο.
Υπολογίζεται αμέσως αν είμαστε στο Zone A ή B και η 
αντίστοιχη τιμή ανά θέση, και η τελική τιμή για τις Χ θέσεις.
Το βάζουμε σε sleep για να περιμένει ο επόμενος πελάτης.
Θέτουμε τις τιμές στο struct: το status, τη ζώνη, το πλήθος 
θέσεων και την τελική τιμή, και υπολογίζεται ο χρόνος 
αναμονής. Κλειδώνουμε το πλάνο για να μην αλλάξει 
“καταλάθος”.
Σε ένα for loop (από το start_row μέχρι το end_row που 
είχαμε αποθηκεύσει προηγουμένως) ελέγχουμε εάν οι 
συνεχόμενες θέσεις που ζητήθηκαν είναι διαθέσιμες. 
Ουσιαστικά για κάθε θέση (plan[i][j]) που δεν την έχει κλείσει 
άλλος πελάτης (==0), αυξάνουμε έναν counter μέχρι να 
βρούμε Χ (seats) διαθέσιμες θέσεις στη σειρά. Aν δεν ισχύει 
το ==0 μέσα στη λούπα για κάποια θέση, ο counter γίνεται 
και πάλι 0 για να ξεκινήσουμε τη διαδικασία από την αρχή. 
Όταν counter==seats, κλείνουμε τις θέσεις για τον πελάτη, 
δεσμεύοντάς τες στο plan με τη σειρά και αποθηκεύοντας 
στο struct μας τα απαραίτητα στοιχεία. Κάνουμε unlock το 
plan, αυξάνουμε τον operator, και τον κάνουμε signal και 
unlock για να πάει στον επόμενο πελάτη-νήμα.
Προσθέτουμε τον χρόνο εξυπηρέτησης (μέχρι τώρα) στον 
t_service.
Note: στο struct το status γίνεται 1 εάν πετύχει η δέσμευση των 
θέσεων, αλλιώς είμαστε στο 0 που σημαίνει ότι απέτυχε η κράτηση 
θέσεων.


4. int pay(…)

Πάλι κάνουμε initialize τους χρόνους αρχής/τέλους όπως
πριν. Περνάμε στον ταμία, που αντίστοιχα γίνεται lock αν 
είναι ελέυθερος ή περιμένουμε μέχρι να γίνει διαθέσιμος. 
Μειώνουμε το cashiers και κάνουμε unlock το mutex του 
(πήρε κάποιον πελάτη). Προσθέτουμε τον χρόνο αναμονής 
για τον πελάτη στο t_waiting. Κάνουμε πάλι sleep για την 
αναμονή. Με χρήση της rand_r και το δοσμένο ποσοστό 
επιτυχίας, γίνεται η πληρωμή, αυξάνεται δηλαδή το bank
account (τα έσοδα) και το status στο struct μας γίνεται 2 
(επιτυχής κράτηση). Διαφορετικά, οι θέσεις που ζήτησε 
αποδεσμεύονται (επιστρέφονται στο plan). Κάνουμε unlock
το plan, αυξάνουμε τον cashier, και τον κάνουμε signal και 
unlock για να πάει στον επόμενο πελάτη-νήμα.
Προσθέτουμε τον χρόνο εξυπηρέτησης που υπολογίσαμε 
μέσα στην pay(), στο t_service.


5. int print_plan()

Με αυτή τη μέθοδο τυπώνουμε ό,τι μας έχει ζητηθεί για 
έξοδος του προγράμματός μας:
-Το plan (οι θέσεις που έχουν δεσμευτεί με κατάλληλη 
σύνταξη)
-Τα συνολικά έσοδα
-Το ποσοστό των συναλλαγών, με τους κατάλληλους 
υπολογισμούς, με τους 3 τρόπους αντίστοιχα (έχουμε 
φροντίσει να αποθηκεύουμε σε 3 counters για το κάθε status
πελάτη)
-Το μέσο χρόνο αναμονής των πελατών (για τα wait με τους 
τηλεφωνητές και ταμίες)
-Το μέσο χρόνο εξυπηρέτησης (εφόσον εξυπηρετήθηκαν 
από τηλεφωνητή και ίσως ταμία)
