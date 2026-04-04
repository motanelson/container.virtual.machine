import copy
print ("\033c\033[40;37m\n")

ss="""8086 x86
80186 x86
80286 x86
80386 x86
80486 x86
arm7 arm
arm8 arm1"""

def finds(s,s1):
    i=0
    ss=s
    l=""
    count=0
    count1=0
    count2=0
    count3=0
    count4=0
    count5=0
    countcc=0
    r=-1
    l=len(ss)
    ll=len(s1)
    if ll<1:
        return -1
    while 1:
        count1=count+1
        if count>l:
            return -1
        a=ss[count:count1]
        
        if a==s1[0:1]:
            if ll==1:
                return count
            count2=1
            count3=2
            count4=count+1
            count5=count4+1
            r=0
            x=True
            while x:
                if count4>l:
                     return -1
                aa=ss[count4:count5]
                
                if aa!=s1[count2:count3]:
                    x=False
                                
                count2=count2+1
                count3=count2+1
                count4=count4+1
                count5=count4+1
                if count2>ll:
                    return count
            count=count+1
        else:
            count=count+1
            
    return -1
l=finds(ss,"x86")
print(l)