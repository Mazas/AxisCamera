import java.util.Random;

public class RSA
{

    private int e;
    private int n;
    private int d;


    public RSA()
    {
        int p = getRandomPrime();
        int q = getRandomPrime();

        n = p * q;

        int phin = (p - 1) * (q - 1);

        for (e = 5; e <= 100000; e++) {
            if (findGCD(phin, e) == 1)
                break;
        }

        for (d = e + 1; d <= 100000; d++) {
            if ( ((d * e) % phin) == 1)
                break;
        }
    }

    private int findGCD(int n1, int n2) {
        int i, gcd =1;

        for(i = 1; (i <= n1) && (i <= n2); ++i) {
            if(((n1 % i) == 0 )&& ((n2 % i) == 0))
                gcd = i;
        }

        return gcd;
    }

    public int powMod(int a, int b, int n) {
        long x = 1, y = a;

        while (b > 0) {
            if ((b % 2) == 1)
                x = (x * y) % n;
            y = (y * y) % n; // Squaring the base
            b /= 2;
        }

        return (int) x % n;
    }
    public byte decrypt(int data){
        return (byte)powMod(data,d,n);
    }

    public String getn(){
        System.out.println("n "+n);
        return String.valueOf(n);
    }

    public String gete(){
        System.out.println("e "+e);
        return String.valueOf(e);
    }

    private int getRandomPrime(){
        int[] primes ={127,131,137,139,149,151,157,163,
                167,173,179,181,191,193,197,199,211,223,
                227,229,233,239,241,251,257,263,269,271};
        Random rng = new Random();
        return primes[rng.nextInt(primes.length-1)];
    }
}
