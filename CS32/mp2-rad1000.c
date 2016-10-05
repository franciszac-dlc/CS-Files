#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define FILLER_SIZE 50
#define RADIX 1000
#define RADIXSIZE 3
// 0 to 999,999,999 (9 digits)
// Base 10
// Circular doubly linked list with list head

// -1 % 10 = 9 not -1
int mod(int a, int b) {
	int c = a % b;
	if(c<0) c += b;
	return c;
}

// For addition / subtraction
int carryover(int a, int b) {
	if(a > 0) return a / b; //Addition
	else return a ? -1 : 0; //Subtraction
}

typedef struct List {
	int data;
	struct List * next;
	struct List * prev;
} List;

// O(1)
List * init() {
	List * out = (List*) malloc(sizeof(List));
	out->data = 0; // Size of the list
	out->next = out;
	out->prev = out;
	return out;
}

// Prepend puts the new digit between the list head and first digit
// Use when reading from highest to lowest place value
// O(1)
List * prepend(List * list, int data) {
	List * newnode = (List*) malloc(sizeof(List));
	newnode->data = data;
	
	newnode->next = list->next;
	newnode->next->prev = newnode;

	list->next = newnode;
	newnode->prev = list;

	//Update size of list
	list->data = list->data + 1;
	return list;
}

// Append puts the new digit at the end
// Use when reading from lowest to highest place value
// O(1)
List * append(List * list, int data) {
	List * tail = list->prev;
	// If new list, set the highest place value
	List * newnode = (List*) malloc(sizeof(List));
	newnode->data = data;
	newnode->next = list;
	list->prev = newnode;

	tail->next = newnode;
	newnode->prev = tail;
	tail = newnode;

	//Update size of list
	list->data = list->data + 1;
	return list;
}

// Delete end removes least significant digits (opposite of prepend)
// O(1)
List * deleteRight(List * list) {
	List * retnode = list->next;
	list->next = retnode->next;
	list->next->prev = list;
	free(retnode);
	list->data -= 1;
	return list;
}

// O(e), e = number of digits of n
// radix is 1,000
List * intToList(int n) {
	// Assume representation in base 10
	List * output = init();
	int modu = 0;
	while(n > 0) {
		// mod = n % RADIX; // Ones digit
		modu = mod(n, RADIX);
		//printf("%d: ", modu);
		//printNumberCorrect(output);
		output = append(output, modu);
		n = n / RADIX; // Remainder
	}
	return output;
}

// Duplicate list
// UNTESTED !!!!!!!!!!
// O(n)
List * duplicate(List * n) {
	List * new = init();
	List * ptr = n->next;
	while(ptr != n) {
		new = append(new, ptr->data);
		ptr = ptr->next;
	}
	return new;
}

// O(n)
List * freeList(List * list) {
	if(list == NULL) return NULL;
	List * ptr = list->next;
	List * head = list;
	list = ptr;
	while(ptr != head) {
		list = list->next;
		free(ptr);
		ptr = list;
	}
	free(ptr);
	return NULL;
}

FILE * readFiller(FILE * fin, char * str) {
	int ndx = 0;
	str[ndx] = (char) fgetc(fin);
	while(ndx < FILLER_SIZE) {
		if(feof(fin)) break;
		ndx++;
		str[ndx] = (char) fgetc(fin);
		if(str[ndx] == ':') {
			ndx++;
			str[ndx] = (char) fgetc(fin);
			break;
		} else if(str[ndx] == '\n') {
			break;
		}
	}
	if(ndx < FILLER_SIZE) str[ndx] = '\0'; //Terminate the input string
	else str[49] = '\0';
	return fin;
}

FILE * readNumber(FILE * fin, List * list) {
	// Declarations
	char stream[] = {'$', '\0'};
	int numstream = -1;
	int radixstream = 0;
	int radix = RADIX / 10;
	while(!feof(fin) && stream[0] != ',' && stream[0] != '\n') {
		// Reset values
		radixstream = 0;
		radix = RADIX/10;
		// Read the radix-1B number
		while(radix > 0) {
			stream[0] = (char) fgetc(fin);
			if(feof(fin)) break;
			//printf("%s:stream, %d:feof\n", stream, feof(fin));
			if(stream[0] == ',' || stream[0] == '\n' || stream[0] == ' '){
				break;
			}
			numstream = atoi(stream);
			if(numstream == 0 && stream[0] != '0') {
				stream[0] = ',';
				break; // Nonnumber detected
			}
			//printf("|%s|%d|%d|\n", stream, numstream, radix);
			radixstream += numstream*radix;
			radix /= 10;
			numstream = 0;
		}
		//printf(":%d:\n", radixstream);

		// Append number to list
		list = prepend(list, radixstream);
	}
	//printf("|%d|\n", radix);

	// A 1000 | n number won't be read anyway; remove trailing zeroes (right)
	List * retnode = list->next;
	while(retnode != list && retnode->data == 0) {
		list->next = retnode->next;
		list->next->prev = list;
		free(retnode);
		retnode = list->next;
		list->data -= 1;
	}

	// Shift the digits if necessary
	radix *= 10;
	if(radix>0 && radix<RADIX) {
		List * shift_ptr = list->next;
		int shiftee = 0;
		int shiftee_offset = RADIX / radix;
		while(shift_ptr != list) {
			shift_ptr->data /= radix;
			if(shift_ptr->next != list) shiftee = shift_ptr->next->data % radix;
			else shiftee = 0;
			shiftee *= shiftee_offset;
			shift_ptr->data += shiftee;
			shift_ptr = shift_ptr->next;
			shiftee = 0;
		}
	}

	// Remove trailing zeroes (left)
	retnode = list->prev;
	while(retnode != list && retnode->data == 0) {
		list->prev = retnode->prev;
		list->prev->next = list;
		free(retnode);
		retnode = list->prev;
		list->data -= 1;
	}
	return fin;
}

FILE * readMessage(FILE * fin, List * list) {
	// Declarations
	int stream = -1;
	stream = fgetc(fin);
	while(stream != 10) { //116: t; 10: \n
		if(feof(fin)) break;
		if(stream > 64 && stream < 91) {
			stream -= 65;
		} else if(stream > 96 && stream < 123) {
			stream -= 97;
		} else if(stream == 32) {
			// space
			stream = 26;
		}
		list = prepend(list, stream);
		stream = fgetc(fin);
	}

	int c = 0;
	// Remove the shit
	if(list->next->data == 1) {
		// remove " to Bob"
		for(;c<7;c++) list = deleteRight(list);
	} else if(list->next->data == 4) {
		// remove " from Alice"
		for(;c<11;c++) list = deleteRight(list);
	}

	//Remove trailing whitespace
	if(list->next->data == 26) {
		List * retnode = list->next;
		list->next = retnode->next;
		list->next->prev = list;
		free(retnode);
		list->data -= 1;
	}
	return fin;
}

void printNumber(List * num) {
	// In reverse
	printf("%d::", num->data);
	List * ptr = num->next;
	while(ptr != num) {
		printf("%d.", ptr->data);
		ptr = ptr->next;
	}
	printf("\n");
	return;
}

void printNumberCorrect(List * num) {
	List * tail = num->prev;
	if(num == tail) {
		printf("%d::0\n", num->data);
		return;
	}

	List * tailptr = tail;
	//printf("%d::", num->data);
	// Print negative
	if(num->data < 0) printf("-");
	while(tailptr != num) {
		// Print zero padding
		if(tailptr != tail) printf("%03d", tailptr->data);
		else printf("%d", tailptr->data);
		tailptr = tailptr->prev;
	}
	printf("\n");
	return;
}

void printMessage(List * msg) {
	printf("%d::", msg->data);
	List * ptr = msg->prev;
	char stream = '%';
	while(ptr != msg) {
		if(ptr->data < 10) {
			printf("%d", ptr->data);
		} else {
			if(ptr->data == 32) stream = ' ';
			else stream = (char) (ptr->data + 55);
			printf("%c", stream);
		}
		ptr = ptr->prev;
	}
	printf("\n");
	return;
}

void printMessageCorrect(List * msg) {
	//printf("%d::", msg->data);
	List * ptr = msg->prev;
	char stream = '%';
	int a = (int) 'A';
	while(ptr != msg) {
		if(ptr->data != 26) {
			printf("%c", (char) (ptr->data + a));
		} else {
			printf(" ");
		}
		ptr = ptr->prev;
	}
	printf("\n");
	return;
}

FILE * printMessageFile(List * msg, FILE * f) {
	//printf("%d::", msg->data);
	List * ptr = msg->prev;
	char stream = '%';
	int a = (int) 'A';
	while(ptr != msg) {
		if(ptr->data != 26) {
			fprintf(f, "%c", (char) (ptr->data + a));
		} else {
			fprintf(f, " ");
		}
		ptr = ptr->prev;
	}
	fprintf(f, "\n");
	return f;
}

// Compares A and B (+ if A is bigger, - if B is bigger, 0 otherwise)
// Om(1) to O(n)
int compare(List * a, List * b) {
	List * atail = a->prev;
	List * btail = b->prev;
	if(a->data != b->data) return a->data - b->data; //Covers the cases when a and b are negative
	if(a->data == 0) return 0;

	while(atail != a) {
		if(atail->data != btail->data) return atail->data - btail->data;
		atail = atail->prev;
		btail = btail->prev;
	}
	return 0;
}

// Computes A + B
// O(i+j) or O(n), A has i digits; B has j digits
List * add(List * a, List * b) {
	//if(a->data == 0) return duplicate(b);
	//if(b->data == 0) return duplicate(a);
	// Instantiate atail and btail
	List * atail = a->prev;
	List * btail = b->prev;
	// Assume both numbers are positive or both are negative
	// Get the signs
	int ka = a->data; // Stores number of digits (pos or neg depending on a)
	int kb = b->data;
	int signa = ka; // Stores abs(ka) first, then the sign of a
	int signb = kb;
	int signsum = 1; // Stores the final sign of the sum

	if(signa < 0) {
		a->data *= -1; // for |a| comparison
		signa *= -1;
	}
	if(signb < 0) {
		b->data *= -1; // for |b| comparison
		signb *= -1;
	}

	int k = signa; // For determining max number of digits
	if(signb > k) k = signb;

	// Faulty comparison of |a| and |b|; change later if needed
	//printf("%d;%d\n", ka, kb);
	//printf("comp: %d\n", compare(a,b));
	if((ka < 0) ^ (kb < 0)) {
		// AB < 0
		if(compare(a, b) > 0) {
			// |a| > |b|
			signa = 1;
			signb = -1;
			if(ka < 0) signsum = -1; //If the bigger number is negative
			else signsum = 1;
		} else {
			// |b| > |a|
			signa = -1;
			signb = 1;
			if(kb < 0) signsum = -1; //If the bigger number is negative
			else signsum = 1;
		}
	} else {
		// AB >= 0
		signa = 1;
		signb = 1;
		if(ka < 0) signsum = -1; //If a and b are negative
		else signsum = 1;
	}
	//printf("ss:%d\n", signsum);

	// Restore values of a->data and b->data from the abs() calc.
	a->data = ka;
	b->data = kb;

	k++;
	int c = 0; // Carryover
	int s = 0; // Sum mod 10
	int i = 0; // Iterator
	int t = 0; // Digit sum
	List * sum = NULL;
	sum = init();
	List * sumtail = sum;
	List * ai = a->next;
	List * bi = b->next;
	int adata = 0;
	int bdata = 0;

	for(; i < k-1; i++) {
		if(ai == a && bi == b) break;
		if(ai == a){
			adata = 0;
		} else {
			adata = ai->data;
			ai = ai->next;
		}

		if(bi == b){
			bdata = 0;
		} else {
			bdata = bi->data;
			bi = bi->next;
		}

		t = (signa*adata) + (signb*bdata) + c;
		// s = t % 10; // Get the digit
		s = mod(t, RADIX);
		// c = t / 10; // Update the carryover
		c = carryover(t, RADIX);
		//printf("%d|%d|%d\n", t, s, c);

		// Append s to the sum
		sum = append(sum, s);
	}

	// Add carryover if it isn't just 0
	if(c) {
		sum = append(sum, c);
	}

	//update signsum
	sum->data *= signsum;

	// Trailing zeroes
	List * retnode = sum->prev;
	while(retnode->data == 0 && retnode != sum) {
		sum->prev = retnode->prev;
		sum->prev->next = sum;
		free(retnode);
		retnode = sum->prev;
		if(sum->data > 0) sum->data -= 1;
		else sum->data += 1;
	}

	return sum;
}

// Computes AB (might be inefficient; can be optimized later)
// O(ij) or O(n^2), A has i digits; B has j digits
List * mult(List * a, List * b) {
	// If one of them is 0
	if(a->data == 0 || b->data == 0) return init();
	// Worry about signs and combing out the product later
	int p = 0; // Product of digits mod 10
	int t = 0; // Product of digits
	int c = 0; // Carryover

	// Keep track of how many ->next's pi and pj will make
	// Size of number = pi + pj
	int isteps = 0; // Steps *pi has made
	int jsteps = 0; // Steps *pj has made

	// Pointers in the list
	List * product = NULL;
	product = init();
	// Initially 0->0->NULL
	product = append(product, 0);
	List * pi = product->next; // Iterator for list a
	List * pj = product->next; // Iterator for list b
	List * ai = a->next; // Iterator along list a
	List * bj = b->next; // Iterator along list b

	while(ai != a) {
		c = 0;
		pj = pi; // Move pj to where pi is now
		bj = b->next;
		while(bj != b) {
			t = (ai->data)*(bj->data) + (pj->data) + c;
			// pj->data = t % 10;
			pj->data = mod(t, RADIX);
			c = t / RADIX;

			//Debugging
			//printf("%dx%d\n", ai->data, bj->data);

			// Move the stuff
			if(pj->next == product) {
				product = append(product, 0);
			}
			pj = pj->next;
			bj = bj->next;
		}
		pj->data += c;

		// Move the stuff
		if(pi->next == product) {
			product = append(product, 0);
		}
		pi = pi->next; // Might be reason for a segfault
		ai = ai->next; // Move to the next digit of a
	}

	// Remove the trailing zero?
	List * producttail = product->prev;
	while(producttail->data == 0) {
		pi = producttail->prev;

		pi->next = product;
		product->prev = pi;
		free(producttail);
		producttail = pi;
		product->data -= 1;
	}

	// Set if positive or negative (NONE YET)
	if((a->data < 0) ^ (b->data < 0)) product->data *= -1;
	return product;
}

List * sp_mult(List * a, int b) {
	// If one of them is 0
	if(a->data == 0 || b == 0) return init();
	// Worry about signs and combing out the product later
	int t = 0; // Product of digits
	int c = 0; // Carryover

	List * product = init();
	List * pj = product;
	List * ai = a->next;
	while(ai != a) {
		t = (ai->data)*b + c;
		c = t / RADIX;
		t = mod(t, RADIX);
		product = append(product, t);
		ai = ai->next;
	}

	if(c != 0) product = append(product, c);
	// Set if positive or negative (NONE YET)
	if((a->data < 0) ^ (b < 0)) product->data *= -1;
	return product;
}

// Computes A - B
// O(n), same as add
List * sub(List * a, List * b) {
	//printNumberCorrect(b);
	int x = b->data;
	x *= -1;
	b->data = x;
	/*printf("Compute a + b: ");
	printNumberCorrect(a);
	printNumberCorrect(b);*/
	List * amb = add(a, b);
	b->data *= -1;
	return amb;
}

// Computes "bitshifting" (-n implies to right, vv.)
// O(n), n being amount shifted
List * shift(List * a, int n) {
	// Just leave if a is already 0
	if(a == NULL || a->data == 0) return a;
	// If shifting to the left (* RADIX)
	while(n > 0) {
		a = prepend(a, 0);
		n--;
	}
	// If shifting to the right (/ RADIX)
	while(n < 0) {
		// Delete least significant digits
		a = deleteRight(a);
		n++;
	}
	return a;
}

// Might be a bottleneck
// To be optimized (storing powers of 27 somewhere)
// O(2n^3 + 6n^2) or O(n^3)
List * base27to10(List * a) {
	List * b = intToList(27);
	List * powerb = intToList(1);
	List * oldpower = powerb; // Holds the old power of b
	List * digita = a->next; // Holds the digit of a (b27)
	List * output = init(); // Holds the output list
	List * oldout = output; // Holds the old output (adding creates a new list)
	List * tempout = NULL; // Holds the partial output (digit * power of b)
	List * hpdigita = NULL; // Holds the list representation of digit of a
	while(digita != a) {
		hpdigita = intToList(digita->data); // Digit of a in list form
		tempout = mult(hpdigita, powerb); // Partial outuput

		output = add(output, tempout); // Add partial output to total output
		freeList(oldout); // Free the old output
		oldout = output; // Set the new old output

		powerb = mult(powerb, b);
		freeList(oldpower);
		oldpower = powerb;
		digita = digita->next; // Get the next digit

		freeList(tempout);
		freeList(hpdigita);
	}

	freeList(powerb);
	freeList(b);
	return output;
}

// O(n log n) ?
// Finds estimate for reciprocal of d times b^2e
// NOTE: in modulus and div, the "exponent" is actually 2(e+1) because of indexing
List * NewtonRhapson(List * d, int two_e) {
	// Generate x0 first
	int power_x0 = d->data;
	if(power_x0 > 0) power_x0 *= -1;
	power_x0 += two_e;
	//power_x0 -= 1; // F A U L T Y M A T H
	List * x0 = intToList(1);
	x0 = shift(x0, power_x0);
	List * two_b2e = intToList(2);
	two_b2e = shift(two_b2e, two_e);
	//printf("precision of recip: %d\n", power_x0);

	// Declare delta_x and other list items
	List * x = NULL;
	List * delta_x = intToList(1);
	List * deltaret = delta_x;
	List * dx0 = NULL;
	List * x0prod = NULL;
	List * x0ret = x0;
	/*printf("precond: (%d:powerx0) ", power_x0);
	printNumberCorrect(x0);
	printf("2*b^2e: ");
	printNumberCorrect(two_b2e);
	printf("denom: ");
	printNumberCorrect(d);*/

	while(delta_x->data != 0) {
		dx0 = mult(d, x0); // d * x0
		x0prod = sub(two_b2e, dx0); // 2b^2e - dx0
		x = mult(x0, x0prod); // x0 * etc.
		//printf("b4shift: ");
		//printNumberCorrect(x);
		x = shift(x, -1*two_e); // div by b^2e
		delta_x = sub(x, x0);
		//printf("iterx: ");
		//printNumberCorrect(x);

		// reset the lists
		x0 = x; // Move x0 up the sequence. as of now x and x0 are same
		freeList(dx0);
		freeList(x0prod);
		freeList(x0ret);
		freeList(deltaret);
		x0ret = x0; //Prepare for freeing
		deltaret = delta_x;
	}

	// return x
	freeList(delta_x);
	freeList(two_b2e);
	return x;
}

// O(n^2 + 2n + n log n) or O(n^2)
// Computes [a/b] with NewtonRhapson
List * divide(List * a, List * b, List * b_recip, int two_k_out) {
	// Check if the answer will be zero
	// fuck it assume it will return 0 if a < b
	int bprec = a->data; // P R E C I S I O N
	int deletThis = 1;
	List * hack = intToList(10);
	List * hack_b_recip = NULL;
	if(bprec < 0) bprec *= -1;
	int k = bprec + 1; // Muh precision
	int two_k = k*2; // Gimme that sweet sweet P R E C I S I O N
	if(b_recip == NULL) {
		b_recip = NewtonRhapson(b, two_k);
		// WARNING: HACK
		hack_b_recip = b_recip;
		b_recip = add(hack_b_recip, hack);
	} else {
		two_k = two_k_out;
		deletThis = 0;
	}
	//printf("100../b = ");
	//printNumberCorrect(b_recip);
	List * adivb = mult(a, b_recip);
	//printf("a*100../b = ");
	//printNumberCorrect(adivb);
	//printf("precision of revert: %d\n", two_k-bprec);
	//printf("adivb? (%d) ", two_k);
	//printNumberCorrect(adivb);
	adivb = shift(adivb, -1*two_k);
	if(deletThis) freeList(b_recip);
	freeList(hack);
	freeList(hack_b_recip);
	return adivb;
}

// O(2n^2) or O(n^2) if q is given
// Does modulo for when barrett can't
List * modulus_alt(List * x, List * m, List * q) {
	// x = qm + r; q = [x/m]
	// r = x - qm;
	int deletThis = 0;
	if(q == NULL) {
		q = divide(x,m,NULL,0);
		deletThis = 1;
	}
	List * qm = mult(q,m);
	List * r = sub(x,qm);
	if(deletThis) freeList(q);
	freeList(qm);
	return r;
}

// O(n) where n is power of radix
// Does modulo but in radix
List * radixModulo(List * x, int power_m) {
	// Basically preserve the power_m least significant digits
	List * ptr = x->next;
	List * output = init();
	while(ptr != x && power_m > 0) {
		output = append(output, ptr->data);
		ptr = ptr->next;
		power_m--;
	}
	return output;
}

// O(2n^2) if x < m^2; O(2n^2 + 8n) if mu is precomputed
// Returns r = x mod m
// Does Barrett Reduction if necessarcy
// BARRETTS ONLY WORKS FOR x < m^2 TAKE NOTE OF THIS
List * modulusBarrett(List * x, List * m, List * mu) {
	int deletThis = 0;
	if(x->data < 0) {
		List * xnew = x;
		List * xret = x;
		while(xnew->data < 0) {
			xnew = add(xnew,m);
			if(xret != x)freeList(xret);
			xret = xnew;
		}
		return xnew;
	}
	if(compare(m, x) >= 0) return duplicate(x); // If x < m
	if(x->data >= 2*(m->data)-1) {
		// Barrett won't work
		//printf("bro you just got Z E E ' D\n");
		//List * output = modulus_alt(x, m, NULL);
		//return output;
	}
	// Get k
	int k = m->data;
	if(k<0) k*=-1;
	int two_k = k * 2;

	// Get mu
	if(mu == NULL) {
		deletThis = 1;
		mu = NewtonRhapson(m, two_k);
	}

	// Instantiate q1, q2, q3, etc.
	List * q1 = duplicate(x);
	q1 = shift(q1, -1*(k-1));
	List * q3 = mult(q1, mu);
	q3 = shift(q3, -1*(k+1));
	//printf("\nq1: ");
	//printNumberCorrect(q1);
	//printf("\nq3: ");
	//printNumberCorrect(q3);
	//printf("\nmu: ");
	//printNumberCorrect(mu);
	//printf("\nm: ");
	//printNumberCorrect(m);
	freeList(q1);
	if(deletThis) freeList(mu);

	List * r1 = radixModulo(x, k+1);
	List * r2 = mult(q3, m);

	/*printf("\nr1: ");
	printNumberCorrect(r1);
	printf("\nr2 ");
	printNumberCorrect(r2);*/

	List * r3 = sub(r1, r2);
	//printf("\nr1: ");
	//printNumberCorrect(r1);
	//printf("\nr2 ");
	//printNumberCorrect(r2);
	//printf("\nr3: ");
	//printNumberCorrect(r3);
	freeList(r1);
	freeList(r2);
	freeList(q3);
	if(r3->data < 0) {
		// This shouldn't happen lol
		List * output = modulus_alt(x,m,NULL);
		freeList(r3);
		return output;
	}

	return r3;
}

List * modulus(List * x, List * m, List * mu) {
	if(x->data < 0) { // If x < 0 (TO BE OPTIMIZED)
		List * xnew = x;
		List * xret = x;
		while(xnew->data < 0) {
			xnew = add(xnew,m);
			if(xret != x)freeList(xret);
			xret = xnew;
		}
		return xnew;
	}
	if(compare(m, x) >= 0) return duplicate(x); // If x < m
	/*
	if(x->data >= 2*(m->data)-1) {
		// Barrett won't work
		//printf("bro you just got Z E E ' D\n");
		List * output = modulus_alt(x, m, NULL);
		return output;
	}
	*/
	List * tmpmod = modulusBarrett(x,m,mu);
	List * tmpret = tmpmod;
	while(compare(tmpmod, m) > 0) {
		// Do barrett again
		tmpmod = modulusBarrett(x,m,mu);
		freeList(tmpret);
		tmpret = tmpmod;
	}
	return tmpmod;
}

// O(3n^3 + 10n^2) or O(n^3) where n = number of digits
// Takes in e and phin s.t. e*x + phin*y = 1, returns x
// THIS MOTHERFUCKER
List * extendedEuclidean(List * e, List * phin) {
	List * t = intToList(0);
	List * newt = intToList(1);
	List * tret = t;
	List * r = duplicate(phin);
	List * newr = duplicate(e);
	List * rret = r;
	List * q = NULL;
	List * qt = NULL;
	List * qr = NULL;
	while(newr->data != 0) {
		// Prepare for freeing
		rret = r;
		tret = t;

		// Do math
		q = divide(r, newr, NULL, 0);
		t = newt;
		qt = mult(q, newt);
		newt = sub(tret, qt);
		r = newr;
		qr = mult(q, newr);
		newr = sub(rret, qr);

		// Free
		freeList(rret);
		freeList(tret);
		freeList(qr);
		freeList(qt);
		freeList(q);
	}
	if(r->data == 1 && r->next->data == 1) {
		// Make t positive
		while(t->data < 0) {
			tret = t;
			t = add(tret, phin);
			freeList(tret);
		}
		freeList(newr);
		freeList(newt);
		freeList(r);
		return t;
	} else {
		//printf("YOU JUST GOT Z E E ' D\n");
		freeList(newr);
		freeList(newt);
		freeList(r);
		freeList(t);
		return NULL; // Or null? Or init()?
	}
}

// O(n^3)
// MONTGOMERY FUNCTIONS
// Precompute ModInv of R mod m
// WEIRDLY SLOWER THAN USUAL
List * modInvOfR(List * m) {
	int power_r = m->data;
	if(power_r < 0) power_r *= -1;
	List * R = intToList(1);
	R = shift(R, power_r);
	//printf("before zee: ");
	//printNumberCorrect(R);
	//printNumberCorrect(m);
	List * Rinv = extendedEuclidean(R, m);
	//if(Rinv == NULL) printf("Z\n");
	freeList(R);
	return Rinv;
}

// O(n^3 + 2n^2 + 5n) or O(n^3)
List * modInvOfMO(List * m) {
	if(m->data == 0) return NULL;
	List * m0 = intToList(m->next->data);
	List * B = intToList(RADIX);
	List * m0_modinv = extendedEuclidean(m0, B);
	if(m0_modinv == NULL) return NULL;
	m0_modinv->data *= -1;
	List * m0_modinvmod = modulus(m0_modinv, B, NULL);
	freeList(m0_modinv);
	freeList(m0);
	freeList(B);
	return m0_modinvmod;
}

// O(
List * changeToMontgo(List * x, List * m, List * MU) {
	//printf("::changeTo::\n");
	int power_r = m->data;
	if(power_r < 0) power_r *= -1;
	List * R = intToList(1);
	R = shift(R, power_r);
	//printf("R = ");
	//printNumberCorrect(R);
	//printf("x = ");
	//printNumberCorrect(x);
	List * Rmod = modulus(R, m, MU);
	//printf("modR = ");
	//printNumberCorrect(Rmod);
	List * Xmod = modulus(x, m, MU);
	//printf("modX = ");
	//printNumberCorrect(x);
	//printNumberCorrect(Xmod);
	List * product = mult(Xmod, Rmod);
	//printf("prod = ");
	//printNumberCorrect(product);
	//printf(" mod = ");
	//printNumberCorrect(m);
	List * productmod = modulus(product, m, MU);
	//printf("prodmod = ");
	//printNumberCorrect(productmod);
	freeList(R);
	freeList(Rmod);
	freeList(Xmod);
	freeList(product);
	//printf("::end changeTo::\n");
	return productmod;
}

List * changeFromMontgo(List * x, List * m, List * MU, List * RINV) {
	List * product = mult(x, RINV);
	List * productmod = modulus(product, m, MU);
	freeList(product);
	return productmod;
}

List * multMontgo(List * xl, List * yl, int power_r, List * m, List * m0_recip) {
	List * productl = mult(xl, yl);
	//printf("::multMongo::\n");
	//printf("montgom unreduced = ");
	//printNumberCorrect(productl);
	List * productl_digit = NULL;
	List * ptr = productl->next;
	List * k = NULL;
	List * mk = NULL;
	List * kmod = NULL;
	List * productlret = NULL;
	int pr = 0, i = 0;
	//printf("%d times\n", power_r);
	for(pr = 0; pr < power_r; pr++) {
		//shift ptr back to the right place
		ptr = productl->next;
		for(i=0; i<pr; i++) {
			if(ptr == productl || ptr->next == productl) break;
			ptr = ptr->next;
		}

		//printf("before: ");
		//printNumberCorrect(productl);
		productlret = productl;
		productl_digit = intToList(ptr->data);
		//printNumberCorrect(productl_digit);
		//printNumberCorrect(m0_recip);
		k = mult(productl_digit, m0_recip);
		kmod = radixModulo(k, 1);
		mk = mult(m, kmod);
		mk = shift(mk, pr);
		productl = add(productlret, mk);
		//printf("LOOOO: \n");
		//printNumberCorrect(productl_digit);
		//printNumberCorrect(k);
		//printNumberCorrect(kmod);
		//printNumberCorrect(mk);
		//printNumberCorrect(productl);

		//free
		freeList(productlret);
		freeList(k);
		freeList(kmod);
		freeList(mk);
		freeList(productl_digit);
	}

	//printf("eeeh: ");
	//printNumberCorrect(productl);
	productl = shift(productl, -1*power_r);
	//printf("noweeeh: ");
	//printNumberCorrect(productl);
	//printf("::end multMongo::\n");
	return productl;
}

// REDO
// Reduce x and y first
List * multMontgoAlt(List * x, List * y, List * m, int power_r, int m_prime) {
	List * A = init();
	List * Aret = A;
	List * A0 = A->next;
	List * xi = x->next;
	//List * xi_digit = NULL;
	//List * ui_digit = NULL;
	List * xiy = NULL;
	List * uim = NULL;
	List * presum = NULL;
	int xi_data = xi->data;
	int y0 = y->next->data;
	int ui = 0;
	int i = 0;
	for(; i<power_r; i++) {
		ui = m_prime*(A0->data + (xi_data * y0));
		ui = ui % RADIX;
		//printf("%d, %d\n", xi_data, ui);
		//xi_digit = intToList(xi_data);
		//ui_digit = intToList(ui);
		//printNumberCorrect(xi_digit);
		//printNumberCorrect(y);
		//xiy = mult(y, xi_digit);
		//uim = mult(m, ui_digit);
		xiy = sp_mult(y, xi_data);
		uim = sp_mult(m, ui);
		presum = add(xiy, uim);
		//printf("xiy: ");
		//printNumberCorrect(xiy);
		//printf("uim: ");
		//printNumberCorrect(uim);
		//printf("presum: ");
		//printNumberCorrect(presum);
		A = add(Aret, presum);
		A = shift(A, -1);

		//free
		//xi_digit = freeList(xi_digit);
		//ui_digit = freeList(ui_digit);
		xiy = freeList(xiy);
		uim = freeList(uim);
		presum = freeList(presum);
		freeList(Aret);

		//set
		Aret = A;
		A0 = A->next;
		xi_data = 0;
		if(xi->next != x) {
			xi = xi->next;
			xi_data = xi->data;
		}
	}

	if(compare(A,m) >= 0) {
		A = sub(Aret, m);
		freeList(Aret);
	}
	return A;
}

// BOIIIIIII
// Returns FINAL PRODUCT
// VERIFICATION???????
List * modExp(List * x, List * e, List * m) {
	// Basic Binary exp
	// 1: Precompute the required constants
	//printf("wewa ");
	//printNumberCorrect(m);
	List * RINV = modInvOfR(m);
	List * MO = modInvOfMO(m);
	int mexp = m->data; // precision of R
	if(mexp<0) mexp*=-1;
	int twoe = mexp * 2; // precision for MU
	List * MU = NewtonRhapson(m, twoe);
	int twoprec = e->data; // precision for div by 2
	if(twoprec < 0) twoprec *= -1;
	twoprec = (twoprec+1) * 2; // precise german engineering
	//List * TWO_RECIP = NewtonRhapson(TWO, twoprec); // Precomputed
	//List * TWO_RECIP_TRUE = add(TWO_RECIP, ONE);
	List * TWO_RECIP = intToList(500);
	List * product = intToList(1); // y
	int lastdig = 0;

	//printf("RINV = ");
	//printNumberCorrect(RINV);
	//printf("MO = ");
	//printNumberCorrect(MO);
	//printf("MU = ");
	//printNumberCorrect(MU);
	//printf("TWORECIP = ");
	//printNumberCorrect(TWO_RECIP_TRUE);

	// 2: Convert x and y in Montgomery form
	List * xl = changeToMontgo(x, m, MU);
	List * xret = xl;
	List * yl = changeToMontgo(product, m, MU);
	List * yret = yl;
	List * eret = e;
	List * etrans = e;
	List * eorig = e;
	//printNumberCorrect(product);
	//printNumberCorrect(x);
	//printf("*** y,x,e ***\n");
	//printNumberCorrect(yl);
	//printNumberCorrect(xl);
	//printNumberCorrect(e);

	// 3: FROOOOT LOOOOOPS
	while(e->data != 0) {
		// Set the free
		xret = xl;
		yret = yl;
		eret = e;
		// Branch if e is even or odd
		lastdig = e->next->data;
		if(lastdig % 2 == 1) { //odd
			//printf("--EVEN--\n");
			// (y,x,n) -> (y, x*x, n/2)
			yl = multMontgo(xret, yret, mexp, m, MO);
			freeList(yret);
			//printf("*** y,x,e ***\n");
			//printNumberCorrect(yl);
			//printNumberCorrect(xl);
			//printNumberCorrect(e);
		}
		xl = multMontgo(xret, xret, mexp, m, MO);
		freeList(xret);
		//e = divide(eret, TWO, TWO_RECIP_TRUE, twoprec);
		// Do the special div by 2 here
		e = mult(eret, TWO_RECIP);
		e = shift(e, -1);
		if(eret != eorig) freeList(eret);
	}

	// 4: Convert yl back to current form
	List * output = changeFromMontgo(yl, m, MU, RINV);
	freeList(xl);
	freeList(yl);
	freeList(e);
	freeList(RINV);
	freeList(MO);
	freeList(MU);
	//freeList(TWO);
	//freeList(ONE);
	freeList(TWO_RECIP);
	//freeList(TWO_RECIP_TRUE);
	freeList(product);
	return output;
}

// REDO
// Reduce x first
List * modExpAlt(List * x, List * e, List * m) {
	// Reduce
	x = modulus(x,m,NULL);
	// Generate R, R2, m'
	int power_r = m->data; // precision of R
	if(power_r<0) power_r*=-1;

	List * BASE = intToList(RADIX);
	List * mmodb = radixModulo(m, 1);
	List * fuck = extendedEuclidean(mmodb, BASE);
	List * this = sub(BASE, fuck);
	int m_prime = this->next->data;

	List * R = intToList(1);
	R = shift(R, power_r);
	List * rmodm = modulus(R, m, NULL);
	List * R2 = mult(rmodm, rmodm);
	List * r2modm = modulus(R2, m, NULL);

	List * x_mont = multMontgoAlt(x, r2modm, m, power_r, m_prime);
	List * A = duplicate(rmodm);
	List * Aret = A;
	List * xret = x_mont;
	List * ONE = intToList(1);
	List * TWO_RECIP = intToList(500);
	List * e_dupe = e;
	List * eret = e;

	// Make a bitlist because t to 0
	clock_t t1 = clock();
	List * bitlist = init();
	while(e_dupe->data != 0) {
		bitlist = prepend(bitlist, e_dupe->next->data % 2);
		e_dupe = mult(eret, TWO_RECIP);
		e_dupe = shift(e_dupe, -1);
		if(eret != e) freeList(eret);
		eret = e_dupe;
	}
	clock_t t2 = clock();
	float ds = ((float)(t2 - t1) / CLOCKS_PER_SEC ) * 1000;
	printf("Time (bitlist): %f\n", ds);

	List * ptr = bitlist->next;

	// For timing
	t1 = clock();
	while(ptr != bitlist) {
		A = multMontgoAlt(A,A,m,power_r,m_prime);
		freeList(Aret);
		Aret = A;
		if(ptr->data == 1) {
			//odd
			A = multMontgoAlt(A,x_mont,m,power_r,m_prime);
			freeList(Aret);
			Aret = A;
		}
		ptr = ptr->next;
	}
	t2 = clock();
	ds = ((float)(t2 - t1) / CLOCKS_PER_SEC ) * 1000;
	printf("Time (exp): %f\n", ds);

	A = multMontgoAlt(Aret,ONE,m,power_r,m_prime);
	freeList(Aret);
	freeList(xret);
	//freeList(x_mont);
	freeList(ONE);
	freeList(BASE);
	freeList(TWO_RECIP);
	freeList(e_dupe);
	freeList(mmodb);
	freeList(fuck);
	freeList(this);
	freeList(R);
	freeList(R2);
	freeList(rmodm);
	freeList(r2modm);
	return A;
}

List * base10to27(List * a) {
	// None yet
	// Precomputeds
	List * base = intToList(27);
	int k = a->data;
	if(k<0) k*=-1;
	k = 2*(k+1);
	List * base_recip = NewtonRhapson(base, k);
	List * output = init();
	List * anext = a;
	List * amod = NULL;

	while(a->data != 0) {
		anext = divide(a, base, NULL, k);
		amod = modulus_alt(a, base, anext);

		//printf("a:");
		//printNumberCorrect(a);
		//printf("anext:");
		//printNumberCorrect(anext);
		//printf("amod:");
		//printNumberCorrect(amod);
		//printf("\n");
		output = append(output, amod->next->data);
		freeList(a);
		a = anext;
		amod = freeList(amod);
	}

	freeList(a);
	freeList(base_recip);
	freeList(base);
	return output;
}

void test() {
	FILE * ftest = fopen("msg.txt", "r");
	List * m = NULL;
	char fil[50];
	while(1) {
		m = init();
		ftest = readFiller(ftest, fil);
		ftest = readMessage(ftest, m);
		if(m->data == 0) break;
		printMessageCorrect(m);
		freeList(m);
	}
	return;
}

void testMem() {
	List * a = intToList(224241);
	List * b = intToList(2011960);
	List * c = changeToMontgo(b,a,NULL);
	freeList(a);
	freeList(b);
	freeList(c);
	return;
}

void testMontgo() {
	List * m = intToList(2101591);
	List * x = intToList(13019);
	List * e = intToList(395);
	/*
	List * ONE = intToList(1);
	List * BASE = intToList(RADIX);
	int mexp = m->data; // precision of R
	if(mexp<0) mexp*=-1;
	int twoe = mexp * 2; // precision for MU
	List * MU = NewtonRhapson(m, twoe);
	int n = mexp;

	List * mmodb = radixModulo(m, 1);
	List * fuck = extendedEuclidean(mmodb, BASE);
	List * this = sub(BASE, fuck);
	int mp = this->next->data;
	List * R = intToList(1);
	R = shift(R, mexp);
	List * Rmod = modulus(R, m, NULL);
	List * R2 = mult(Rmod, Rmod);
	List * R2mod = modulus(R2, m, NULL);
	printf("mp=%d\n", mp);
	*/

	List * montgo = modExpAlt(x,e,m);
	printNumberCorrect(montgo);
	freeList(x);
	freeList(e);
	freeList(m);
	freeList(montgo);
	/*
	freeList(toM);
	freeList(fromM);
	freeList(R);
	freeList(Rmod);
	freeList(R2mod);
	freeList(ONE);
	*/
	return;
}

void testMontRed() {
	List * m = intToList(910851);
	List * x = intToList(10582);
	List * BASE = intToList(RADIX);
	int mexp = m->data; // precision of R
	if(mexp<0) mexp*=-1;
	int twoe = mexp * 2; // precision for MU
	List * MU = NewtonRhapson(m, twoe);
	int n = mexp;
	List * fuck = extendedEuclidean(m, BASE);
	List * this = sub(BASE, fuck);
	int mp = this->next->data;
	List * R = intToList(1000000);
	List * Rmod = modulus(R, m, NULL);
	printf("mp=%d\n", mp);
	printNumberCorrect(Rmod);
	List * R2 = mult(Rmod,Rmod);
	List * R2mod = modulus(R2, m, NULL);
	printNumberCorrect(R2mod);
	List * x_mont1 = changeToMontgo(x,m,MU);
	printf("x1=");
	printNumberCorrect(x_mont1);
	List * x_mont2 = multMontgoAlt(x,R2mod,m,mexp,mp);
	printf("x2=");
	printNumberCorrect(x_mont2);
}

void testexp() {
	List * x = intToList(1023);
	List * e = intToList(39);
	List * m = intToList(9923);
	List * a = modExp(x,e,m);
	printNumberCorrect(a);
	List * b = modExpAlt(x,e,m);
	printNumberCorrect(b);
	return;
}

void testMod() {
	FILE * ftest = fopen("tmod.txt", "r");
	List * x = init();
	List * m = init();
	ftest = readNumber(ftest, x);
	ftest = readNumber(ftest, m);
	List * xm = modulus(x,m,NULL);
	printNumberCorrect(xm);
	return;
}

void testspm() {
	List * a = intToList(235101);
	int b = 204;
	List * c = sp_mult(a,b);
	printNumberCorrect(c);
	return;
}

void main() {
	// Declarations
	FILE * fin;
	FILE * fout;
	fin = fopen("INPUT.txt", "r");
	fout = fopen("201508086.txt", "w");

	char filler[FILLER_SIZE]; //Stores the useless text (eg. Alice / Bob / etc.)
	filler[0] = 'Z';
	List * one = init();
	one = prepend(one, 1);
	List * p = NULL;
	List * pmo = NULL;
	List * q = NULL;
	List * qmo = NULL;
	List * e = NULL;
	List * m = NULL;
	List * mbten = NULL;
	List * n = NULL;
	List * phin = NULL;
	List * d = NULL;
	List * m_raised = NULL;
	List * m_conv = NULL;

	char CASE[10];
	
	while(filler[0] != 'E') {
		fin = readFiller(fin, filler);
		//printf("/%s\\\n", filler);
		if(filler[0] == 'C') {
			fprintf(fout, "%s\n", filler);
			//Case input
			// Reset p,q,e
			p = freeList(p);
			q = freeList(q);
			e = freeList(e);
			n = freeList(n);
			d = freeList(d);
			p = init();
			q = init();
			e = init();
			//pmo = init();
			//qmo = init();
			//n = init();

			// Parse p,q,e
			fin = readNumber(fin, p);
			fin = readNumber(fin, q);
			fin = readNumber(fin, e);

			//Check if tama pagkabasa
			//printf("/%s\\\n", filler);
			//printNumberCorrect(p);
			//printNumberCorrect(q);
			//printNumberCorrect(e);
			//printf("\\******/\n");

			// Get n
			n = mult(p,q);
			//printf("n = ");
			//printNumberCorrect(n);

			// Get phi(n)
			pmo = sub(p, one);
			qmo = sub(q, one);
			//printf("p-1 = ");
			//printNumberCorrect(pmo);
			//printf("q-1 = ");
			//printNumberCorrect(qmo);
			phin = mult(pmo, qmo);
			//printf("phi(n) = ");
			//printNumberCorrect(phin);

			// Get d
			d = extendedEuclidean(e, phin);
			if(d == NULL) {
				fprintf(fout, "Invalid encryption key\n");
				pmo = freeList(pmo);
				qmo = freeList(qmo);
				phin = freeList(phin);
				continue;
			}
			//printf("d = ");
			//printNumberCorrect(d);
			pmo = freeList(pmo);
			qmo = freeList(qmo);
			phin = freeList(phin);
		} else if(filler[0] == 'B') {
			if(d==NULL) continue;
			// Decrypt m
			//printf("Decrypt X\n");
			// Get X
			m = init();
			fin = readMessage(fin, m);
			//printMessageCorrect(m);

			// Convert to b10
			mbten = base27to10(m);
			m_raised = modExpAlt(mbten, d, n);
			m_conv = base10to27(m_raised);
			//printf("m^d mod n b10: ");
			fprintf(fout, "Bob received: ");
			fout = printMessageFile(m_conv, fout);

			// free
			freeList(mbten);
			freeList(m_conv);
			freeList(m);
		} else if(filler[0] == 'A') {
			if(d==NULL) continue;
			// Encrypt m
			//printf("Encrypt M\n");
			// Get M
			m = init();
			fin = readMessage(fin, m);
			//printMessageCorrect(m);
			// Convert to b10
			mbten = base27to10(m);
			m_raised = modExpAlt(mbten, e, n);
			m_conv = base10to27(m_raised);
			//printf("m^e mod n b10: ");
			fprintf(fout, "Alice sent: ");
			fout = printMessageFile(m_conv, fout);

			// free
			freeList(mbten);
			freeList(m_conv);
			freeList(m);
		} else if(filler[0] == 'E') {
			// END
		}
		if(feof(fin)) break;
	}

	//printf("end/%s\\\n", filler);
	// Ending
	p = freeList(p);
	q = freeList(q);
	e = freeList(e);
	n = freeList(n);
	one = freeList(one);

	freeList(pmo);
	freeList(qmo);
	freeList(phin);
	freeList(d);
	fclose(fin);
	fclose(fout);
}
