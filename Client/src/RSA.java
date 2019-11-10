import java.util.Random;

public class RSA
{

    private int e;
    private int n;
    private int d;


    public RSA()
    {
        int p = 19;
        int q = 17;

        n = p * q;

        int phin = (p - 1) * (q - 1);

        int e = 0;
        for (e = 5; e <= 100000; e++) {
            if (findGCD(phin, e) == 1)
                break;
        }

        int d = 0;
        for (d = e + 1; d <= 100000; d++) {
            if ( ((d * e) % phin) == 1)
                break;
        }
    }

    private int checkPrime(int n) {
        int i;
        int m = n / 2;

        for (i = 2; i <= m; i++) {
            if (n % i == 0) {
                return 0; // Not Prime
            }
        }

        return 1; // Prime
    }

    private int findGCD(int n1, int n2) {
        int i, gcd =1;

        for(i = 1; i <= n1 && i <= n2; ++i) {
            if(n1 % i == 0 && n2 % i == 0)
                gcd = i;
        }

        return gcd;
    }

    public int powMod(int a, int b, int n) {
        long x = 1, y = a;

        while (b > 0) {
            if (b % 2 == 1)
                x = (x * y) % n;
            y = (y * y) % n; // Squaring the base
            b /= 2;
        }

        return (int) x % n;
    }
    public byte decrypt(int data){
        return (byte)powMod(data,d,n);
    }

    public String getPQ(){
        return String.valueOf(n);
    }

    public String getEx(){
        return String.valueOf(d);
    }

    private int getRandomPrime(){
        int[] primes ={127,131,137,139,149,151,157,163,
                167,173,179,181,191,193,197,199,211,223,
                227,229,233,239,241,251,257,263,269,271,
                277,281,283,293,307,311,313,317,331,337,
                347,349,353,359,367,373,379,383,389,397,
                401,409,419,421,431,433,439,443,449,457,
                461,463};
        Random rng = new Random();
        return primes[rng.nextInt(primes.length-1)];
    }
}
