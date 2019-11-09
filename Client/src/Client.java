import javax.imageio.ImageIO;
import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.*;
import java.math.BigInteger;
import java.net.*;;
import java.util.Scanner;

public class Client {

    public static void main(String[] args) throws Exception {
        // get input
        Scanner in = new Scanner(System.in);
        System.out.print("ip: ");
        String ipAddress = in.nextLine();
        System.out.print("port: ");
        String port = in.nextLine();
        System.out.print("resolution: ");
        String resolution = in.nextLine();
        System.out.print("fps: ");
        String fps = in.nextLine();
        try {
            // build the string
            String message = "resolution=1280x960&fps=10";
            // building java swing window
            JPanel jPanel = new JPanel();
            jPanel.setBorder(new EmptyBorder(0, 0, 0, 0));
            jPanel.setLayout(null);
            JFrame jFrame = new JFrame();
            JLabel label = new JLabel();
            jFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            ImageIcon icon;
            Socket socket = new Socket("192.168.20.252", 1025);
            PrintWriter outToServer = new PrintWriter(socket.getOutputStream());
            BufferedReader inFromServer = new BufferedReader( new InputStreamReader(socket.getInputStream()));
            int frames = 0;
            long prevTime = System.currentTimeMillis();

            RSA rsa = new RSA();
            outToServer.write(rsa.getEx());
            outToServer.flush();
            System.out.println(inFromServer.readLine());
            outToServer.write(rsa.getPQ());
            outToServer.flush();
            System.out.println(inFromServer.readLine());
            byte xor_key = 101;//Byte.parseByte(rsa.decryptMessage(inFromServer.readLine().getBytes()));
            //send ack
            System.out.println(xor_key);
            outToServer.write("key received");
            outToServer.flush();



            while (true) {
                // send message
                outToServer.write(message);
                outToServer.flush();
                jFrame.setVisible(true);

                int size = Integer.parseInt(inFromServer.readLine());

                //send ack
                System.out.println(size);
                outToServer.write("file size received");
                outToServer.flush();

                byte[] data = new byte[size];
                socket.getInputStream().readNBytes(data,0,size);

                outToServer.write("file received");
                outToServer.flush();

                //decrypt
                for (int i = 0; i<data.length; i++){
                    data[i]= (byte) (data[i]^ xor_key);
                }

                BufferedImage image = ImageIO.read(new ByteArrayInputStream(data));
                System.out.println("Received " + image.getHeight() + "x" + image.getWidth() + ": " + System.currentTimeMillis());

                // display image in the window
                label.setIcon(new ImageIcon(image));
                jFrame.setLayout(new FlowLayout());
                jFrame.setSize(image.getWidth(), image.getHeight());
                jFrame.add(label);
                frames++;
                // calculate frame rate
                if (System.currentTimeMillis() - prevTime > 1000) {
                    jFrame.setTitle(frames + " FPS");
                    prevTime = System.currentTimeMillis();
                    frames = 0;
                }
            }
        }catch (Exception e){
            System.err.println(e.getMessage());
        }
        //socket.close();
    }
}
